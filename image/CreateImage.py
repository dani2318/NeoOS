import argparse
import os
import re
from io import SEEK_CUR, SEEK_SET
from pathlib import Path
import sys
import parted  # Requires Linux or WSL, won't work natively on Windows
import sh      # Same for sh; Windows may require WSL or equivalent

SECTOR_SIZE = 512

def generate_image_file(target: str, size_sectors: int):
    with open(target, 'wb') as fout:
        fout.write(bytes(size_sectors * SECTOR_SIZE))

def create_filesystem(target: str, filesystem, reserved_sectors=0, offset=0):
    if filesystem in ['fat12', 'fat16', 'fat32']:
        reserved_sectors += 1
        if filesystem == 'fat32':
            reserved_sectors += 1

        mkfs_fat = sh.Command('mkfs.fat')
        mkfs_fat(target,
                 F=filesystem[3:],  # fat size (12,16,32)
                 n='NBOS',
                 R=reserved_sectors,
                 offset=offset
        )
    elif filesystem == 'ext2':
        mkfs_ext2 = sh.Command('mkfs.ext2')
        mkfs_ext2(target,
                  L='NBOS',
                  E=f'offset={offset * SECTOR_SIZE}'
        )
    else:
        raise ValueError('Unsupported filesystem ' + filesystem)

from typing import Union
from pathlib import Path

def find_symbol_in_map_file(map_file: Union[str, Path], symbol: str):
    # Converti a Path se è stringa
    map_path = Path(map_file)
    with map_path.open('r') as fmap:
        for line in fmap:
            if symbol in line:
                match = re.search('0x([0-9a-fA-F]+)', line)
                if match is not None:
                    return int(match.group(1), base=16)
    return None


def install_stage1(target: str, stage1: str, stage2_offset, stage2_size, offset=0):
    map_file = Path(stage1).with_suffix('.map')
    if not map_file.exists():
        raise ValueError(f"Can't find {map_file}")

    entry_offset = find_symbol_in_map_file(map_file, '__entry_start')
    if entry_offset is None:
        raise ValueError(f"Can't find __entry_start symbol in map file {map_file}")
    entry_offset -= 0x7c00

    stage2_start = find_symbol_in_map_file(map_file, 'stage2_location')
    if stage2_start is None:
        raise ValueError(f"Can't find stage2_location symbol in map file {map_file}")
    stage2_start -= 0x7c00

    # On Windows, os.open flags work but may differ in behavior; this is fine for binary write
    with open(stage1, 'rb') as fstage1, \
         os.fdopen(os.open(target, os.O_WRONLY | os.O_CREAT | os.O_BINARY if hasattr(os, "O_BINARY") else 0), 'wb+') as ftarget:
        ftarget.seek(offset * SECTOR_SIZE, SEEK_SET)

        ftarget.write(fstage1.read(3))

        fstage1.seek(entry_offset - 3, SEEK_CUR)
        ftarget.seek(entry_offset - 3, SEEK_CUR)
        ftarget.write(fstage1.read())

        ftarget.seek(offset * SECTOR_SIZE + stage2_start, SEEK_SET)
        ftarget.write(stage2_offset.to_bytes(4, 'little'))
        ftarget.write(stage2_size.to_bytes(1, 'little'))

def install_stage2(target: str, stage2: str, offset=0, limit=None):
    with open(stage2, 'rb') as fstage2, \
         os.fdopen(os.open(target, os.O_WRONLY | os.O_CREAT | os.O_BINARY if hasattr(os, "O_BINARY") else 0), 'wb+') as ftarget:
        ftarget.seek(offset * SECTOR_SIZE, SEEK_SET)
        ftarget.write(fstage2.read())

def create_mtools_config(image: str, offset_bytes: int):
    config_content = f'drive x: file="{os.path.abspath(image)}" offset={offset_bytes}\n'
    config_file = f"{image}.mtools.conf"
    with open(config_file, 'w') as f:
        f.write(config_content)
    return config_file

def copy_files_with_mtools(image: str, files, base_dir: str, offset=0):
    if offset > 0:
        config_file = create_mtools_config(image, offset * SECTOR_SIZE)
        mtools_env = os.environ.copy()
        mtools_env['MTOOLSRC'] = config_file
        drive = 'x:'
    else:
        mtools_env = None
        drive = image
        config_file = None

    try:
        for file in files:
            file_src = os.path.abspath(os.path.join(base_dir, "image", file))
            file_rel = os.path.relpath(file_src, base_dir)
            file_dst = f'{drive}/{file_rel}' if offset > 0 else f'::{file_rel}'

            if os.path.isdir(file_src):
                print(f'    ... creating directory {file_rel}')
                if mtools_env:
                    sh.mmd(file_dst, _env=mtools_env)
                else:
                    sh.mmd('-i', drive, file_dst)
            else:
                print(f'    ... copying {file_rel}')
                parent_dir = os.path.dirname(file_dst)
                if parent_dir and parent_dir not in ['', drive, '::']:
                    try:
                        if mtools_env:
                            sh.mmd(parent_dir, _env=mtools_env)
                        else:
                            sh.mmd('-i', drive, parent_dir)
                    except sh.ErrorReturnCode:
                        pass

                if mtools_env:
                    sh.mcopy(file_src, file_dst, _env=mtools_env)
                else:
                    sh.mcopy('-i', drive, file_src, file_dst)
    finally:
        if config_file and os.path.exists(config_file):
            os.remove(config_file)

def build_floppy(image, stage1, stage2, kernel, files, base_dir):
    size_sectors = 2880
    stage2_size = os.stat(stage2).st_size
    stage2_sectors = (stage2_size + SECTOR_SIZE - 1) // SECTOR_SIZE

    generate_image_file(image, size_sectors)

    print("> formatting file using fat12...")
    create_filesystem(image, 'fat12', reserved_sectors=stage2_sectors)

    print("> installing stage1...")
    install_stage1(image, stage1, stage2_offset=1, stage2_size=stage2_sectors)

    print("> installing stage2...")
    install_stage2(image, stage2, offset=1)

    print("> copying files...")
    print(f'    ... copying {kernel}')
    sh.mmd('-i', image, "::boot")
    sh.mcopy('-i', image, kernel, "::boot/")

    copy_files_with_mtools(image, files, base_dir)

def create_partition_table(target: str, align_start: int):
    device = parted.getDevice(target)
    disk = parted.freshDisk(device, 'msdos')
    freeSpace = disk.getFreeSpaceRegions()
    partitionGeometry = parted.Geometry(device, align_start, end=freeSpace[-1].end)
    partition = parted.Partition(disk=disk, type=parted.PARTITION_NORMAL, geometry=partitionGeometry)
    partition.setFlag(parted.PARTITION_BOOT)
    disk.addPartition(partition, constraint=device.optimalAlignedConstraint)
    disk.commit()

def build_disk(image, stage1, stage2, kernel, files, base_dir, image_size, image_fs):
    size_sectors = (image_size + SECTOR_SIZE - 1) // SECTOR_SIZE
    partition_offset = 2048

    stage2_size = os.stat(stage2).st_size
    stage2_sectors = (stage2_size + SECTOR_SIZE - 1) // SECTOR_SIZE

    generate_image_file(image, size_sectors)

    print("> creating partition table...")
    create_partition_table(image, partition_offset)

    print(f"> formatting file using {image_fs}...")
    create_filesystem(image, image_fs, offset=partition_offset)

    print("> installing stage1...")
    install_stage1(image, stage1, offset=partition_offset, stage2_offset=1, stage2_size=stage2_sectors)

    print("> installing stage2...")
    install_stage2(image, stage2, offset=1)

    print("> copying files...")

    config_file = create_mtools_config(image, partition_offset * SECTOR_SIZE)
    mtools_env = os.environ.copy()
    mtools_env['MTOOLSRC'] = config_file

    try:
        print(f"    ... copying kernel...")
        sh.mmd("x:/boot", _env=mtools_env)
        sh.mcopy(kernel, "x:/boot/", _env=mtools_env)

        copy_files_with_mtools(image, files, base_dir, offset=partition_offset)
    finally:
        if os.path.exists(config_file):
            os.remove(config_file)

def build_image(target, source, base_dir, image_type, image_size=None, image_fs=None):
    stage1 = str(source[0])
    stage2 = str(source[1])
    kernel = str(source[2])
    files = source[3:]

    image = str(target[0])
    if image_type == 'floppy':
        build_floppy(image, stage1, stage2, kernel, files, base_dir)
    elif image_type == 'disk':
        if image_size is None or image_fs is None:
            raise ValueError("image_size and image_fs required for disk image")
        build_disk(image, stage1, stage2, kernel, files, base_dir, image_size, image_fs)
    else:
        raise ValueError('Unknown image type ' + image_type)


def main():
    parser = argparse.ArgumentParser(description="Build disk/floppy image")

    parser.add_argument("image_type", choices=["floppy", "disk"], help="Type of image to build")
    parser.add_argument("target", help="Output image file path")
    parser.add_argument("stage1", help="Stage1 binary file")
    parser.add_argument("stage2", help="Stage2 binary file")
    parser.add_argument("kernel", help="Kernel file")
    parser.add_argument("--files", nargs="*", default=[], help="Additional files to include")
    parser.add_argument("--base_dir", default=".", help="Base directory for source files")
    parser.add_argument("--image_size", type=int, default=None, help="Image size in bytes (required for disk)")
    parser.add_argument("--image_fs", default=None, help="Filesystem type (required for disk)")

    args = parser.parse_args()

    source_files = [args.stage1, args.stage2, args.kernel] + args.files
    target = [args.target]

    if args.image_type == "disk":
        if args.image_size is None or args.image_fs is None:
            print("Error: --image_size and --image_fs are required for disk image", file=sys.stderr)
            sys.exit(1)
        build_image(target, source_files, args.base_dir, args.image_type, args.image_size, args.image_fs)
    else:
        build_image(target, source_files, args.base_dir, args.image_type)

if __name__ == "__main__":
    main()