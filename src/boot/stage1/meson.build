# Stage 1 Bootloader Build Configuration
# =======================================
# The first stage bootloader - loaded by BIOS/UEFI

message('Building stage1 bootloader...')

# === Configuration ===
stage1_dir = meson.current_source_dir()
stage2_dir = meson.project_source_root() / 'src' / 'boot' / 'stage2'

# Validate filesystem parameter
valid_filesystems = ['fat12', 'fat16', 'fat32']
if image_fs not in valid_filesystems
    error('Unsupported filesystem for stage1: @0@. Supported: @1@'.format(
        image_fs, ', '.join(valid_filesystems)))
endif

message('Stage1 configuration:')
message('  Target filesystem: ' + image_fs)
message('  Stage2 include dir: ' + stage2_dir)

# === Assembly Source ===
stage1_asm_source = files('nbootloader.asm')[0]

# Verify source file exists
if not import('fs').exists(stage1_asm_source)
    error('Stage1 source file not found: ' + stage1_asm_source.full_path())
endif

# === Build Assembly Object ===
stage1_asm_obj = custom_target(
    'stage1_nbootloader_obj',
    input: stage1_asm_source,
    output: 'nbootloader.o',
    command: [
        nasm,
        '-f', 'elf32',
        '-DFILESYSTEM=' + image_fs.to_upper(),
        '-I', stage2_dir,
        '-o', '@OUTPUT@',
        '@INPUT@'
    ],
    install: false,
)

# === Linker Configuration ===
stage1_linker_script = stage1_dir / 'linker.ld'

# Verify linker script exists
if not import('fs').exists(stage1_linker_script)
    error('Stage1 linker script not found: ' + stage1_linker_script)
endif

stage1_link_args = [
    '-T', stage1_linker_script,
    '-Map', meson.current_build_dir() / 'stage1.map',
    '-nostdlib',
    '-static',
]

# === Build ELF Executable ===
stage1_elf = executable(
    'stage1.elf',
    stage1_asm_obj,
    link_args: stage1_link_args,
    install: false,
    native: false,  # This is a cross-compiled target
)

# === Create Raw Binary ===
stage1_bin = custom_target(
    'stage1.bin',
    input: stage1_elf,
    output: 'stage1.bin',
    command: [
        objcopy,
        '-O', 'binary',
        '@INPUT@',
        '@OUTPUT@'
    ],
    install: false,
    build_by_default: true,
)

# Validate binary size (bootloader must fit in boot sector)
stage1_size_check = custom_target(
    'stage1_size_check',
    input: stage1_bin,
    output: 'stage1_size_check.txt',
    command: [
        python3, '-c',
        '''
import sys, os
size = os.path.getsize(sys.argv[1])
max_size = 510  # 512 bytes - 2 bytes for boot signature
if size > max_size:
    print(f"ERROR: Stage1 bootloader too large: {size} bytes (max: {max_size})")
    sys.exit(1)
else:
    print(f"Stage1 bootloader size OK: {size} bytes")
    with open(sys.argv[2], 'w') as f:
        f.write(f"Stage1 size: {size} bytes\\n")
        ''',
        '@INPUT@', '@OUTPUT@'
    ],
    depends: [stage1_bin],
    console: true,
    build_by_default: true,
)

message('Stage1 bootloader build configured successfully')