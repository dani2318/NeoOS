message('Building stage1 bootloader...')

# Get filesystem from options 
image_fs = get_option('imageFS')

valid_filesystems = ['fat12', 'fat16', 'fat32']
if image_fs not in valid_filesystems
  error('Unsupported filesystem for stage1: @0@. Supported: @1@'.format(
      image_fs, ', '.join(valid_filesystems)))
endif

stage1_dir = meson.current_source_dir()
stage2_dir = meson.project_source_root() / 'src' / 'boot' / 'stage2'

message('Stage1 configuration:')
message('  Target filesystem: ' + image_fs)
message('  Stage2 include dir: ' + stage2_dir)

# Assembly source as files() array
stage1_asm_source = files('nbootloader.asm')

# Verify source file exists
if not import('fs').exists(stage1_asm_source[0].full_path())
  error('Stage1 source file not found: ' + stage1_asm_source[0].full_path())
endif

image_fs_uc = image_fs.to_upper()

filesystems = {
  'FAT12': 1,
  'FAT16': 2,
  'FAT32': 3,
  'EXT2': 4,
}

filesystem_val = filesystems.get(image_fs_uc, 0)

if filesystem_val == 0
  error('Unsupported filesystem: ' + image_fs)
endif

stage1_asm_obj = custom_target(
  'stage1_nbootloader_obj',
  input: stage1_asm_source,
  output: 'nbootloader.o',
  command: [
    nasm,
    '-f', 'elf32',
    '-DFILESYSTEM=' + filesystem_val.to_string(),
    '-I', stage2_dir,
    '-o', '@OUTPUT@',
    '@INPUT@'
  ],
  install: false,
)

stage1_linker_script = stage1_dir / 'linker.ld'
if not import('fs').exists(stage1_linker_script)
  error('Stage1 linker script not found: ' + stage1_linker_script)
endif

# Use project toolchain paths (without .exe extensions)
toolchain_base = meson.project_source_root() / 'toolchain' / 'i686-elf'
ld_prog = find_program(toolchain_base / 'bin' / 'i686-elf-ld', required: false)

# Fallback to system tools if toolchain not found
if not ld_prog.found()
  ld_prog = find_program('i686-elf-ld', required: true)
endif

message('Using linker: ' + ld_prog.full_path())

# Use direct linker instead of executable() to avoid host system targeting
stage1_elf = custom_target(
  'stage1_elf',
  input: stage1_asm_obj,
  output: 'stage1.elf',
  command: [
    ld_prog,
    '-T', stage1_linker_script,
    '-Map=' + meson.current_build_dir() + '/stage1.map',
    '-m', 'elf_i386',  # Force 32-bit ELF output
    '-static',
    '-nostdlib',
    '-o', '@OUTPUT@',
    '@INPUT@'
  ],
  install: false,
  build_by_default: true,  # ensure .elf is built by default
)

message('Stage1 bootloader build configured successfully')