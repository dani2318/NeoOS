# NeoOS Build System
# ===================
# This build file is designed to be used with a Meson cross-file,
# for example, named 'i686-elf.cross', that defines the toolchain compilers.
# 
# To configure the project, run:
# meson setup build --cross-file i686-elf.cross -Dconfig=release
# The `meson setup` command will read options from meson_options.txt.

# Require minimum Meson version for stability

project('NeoOS', 'c', 'cpp',
    version: '0.1.0',
    license: 'AGPL-3.0 license',
    default_options: [
        'c_std=gnu11',
        'cpp_std=c++17',
        'warning_level=2',
        'buildtype=' + get_option('config'),
        'default_library=static',
    ]
)

meson_version = meson.version()
if meson_version.version_compare('<0.50.0')
    error('Meson version 0.50.0 or newer is required, found: ' + meson_version)
endif



# Display build configuration
message('Building NeoOS version: ' + meson.project_version())
message('Build configuration: ' + get_option('config'))
message('Cross compiling: ' + (meson.is_cross_build() ? 'yes' : 'no'))

# === Compiler and Tool Detection ===
cc = meson.get_compiler('c')
cxx = meson.get_compiler('cpp')

# Find required tools
python3 = find_program('python3', required: true)
nasm = find_program('nasm', required: true)
objcopy = find_program('objcopy', required: true)
strip_prog = find_program('strip', required: true)

# Optional tools for running/debugging
qemu = find_program('qemu-system-i386', required: false)
gdb = find_program('gdb', required: false)

if not qemu.found()
    warning('QEMU not found - run target will not work')
endif

# === Compiler Flags ===
base_cflags = [
    '-ffreestanding',
    '-nostdlib',
    '-fno-builtin',
    '-fno-stack-protector',
    '-nostdinc',
    '-m32',
    '-march=i686',
    '-Wall',
    '-Wextra',
    '-Werror=implicit-function-declaration',
]

base_cxxflags = base_cflags + [
    '-fno-exceptions',
    '-fno-rtti',
    '-fno-threadsafe-statics',
]

# Debug vs Release flags
if get_option('config') == 'release'
    optimization_flags = ['-O2', '-DNDEBUG']
    message('Release build: optimizations enabled')
else
    optimization_flags = ['-O0', '-g', '-DDEBUG']
    message('Debug build: debug symbols enabled')
endif

# Final compiler flags
cflags = base_cflags + optimization_flags
cxxflags = base_cxxflags + optimization_flags

# === Image Configuration ===
image_type = get_option('imageType')
image_fs = get_option('imageFS')
image_size = get_option('imageSize')

# Validate image configuration
valid_image_types = ['hdd', 'floppy']
if image_type not in valid_image_types
    error('Invalid image type: @0@. Must be one of: @1@'.format(image_type, ', '.join(valid_image_types)))
endif

# Override filesystem for floppy images
if image_type == 'floppy'
    if image_fs != 'fat12'
        message('Forcing FAT12 filesystem for floppy image')
        image_fs = 'fat12'
    endif
endif

message('Image configuration:')
message('  Type: ' + image_type)
message('  Filesystem: ' + image_fs)
message('  Size: ' + image_size)

# === Project Structure ===
src_dir = meson.project_source_root() / 'src'
scripts_dir = meson.project_source_root() / 'scripts'
image_root_dir = meson.project_source_root() / 'image' / 'root'

# === Build Subdirectories ===
message('Building components...')

# Build core library first (required by other components)
subdir('src/libs/core')

# Build bootloader stages
subdir('src/boot/stage1')
subdir('src/boot/stage2')

# Build kernel
subdir('src/kernel')

# === Final Disk Image ===
message('Configuring disk image creation...')

create_image_script = meson.project_source_root() / 'image' / 'CreateImage.py'

# Verify the image creation script exists
if not import('fs').exists(create_image_script)
    error('Image creation script not found: ' + create_image_script)
endif

# Create the bootable disk image
neoos_image = custom_target(
    'NeoOS.img',
    input: [kernel_elf, stage1_bin, stage2_bin],
    output: 'NeoOS.img',
    command: [
        python3, create_image_script,
        '--kernel', '@INPUT0@',
        '--stage1', '@INPUT1@',
        '--stage2', '@INPUT2@',
        '--image-type', image_type,
        '--image-fs', image_fs,
        '--image-size', image_size,
        '--root-dir', image_root_dir,
        '--output', '@OUTPUT@',
    ],
    console: true,
    build_by_default: true,
    install: false,
)

# === Development Targets ===
run_script = scripts_dir / 'run.sh'
debug_script = scripts_dir / 'debug.sh'

# Run target - boot the OS in QEMU
if qemu.found() and import('fs').exists(run_script)
    run_target = custom_target(
        'run',
        command: [find_program('sh'), run_script, image_type, neoos_image],
        output: 'run_dummy',
        depends: [neoos_image],
        console: true,
        build_always_stale: true,
        install: false,
    )
endif

# Debug target - boot the OS in QEMU with GDB
if qemu.found() and gdb.found() and import('fs').exists(debug_script)
    debug_target = custom_target(
        'debug',
        command: [find_program('sh'), debug_script, image_type, neoos_image],
        output: 'debug_dummy',
        depends: [neoos_image],
        console: true,
        build_always_stale: true,
        install: false,
    )
endif

# === Build Summary ===
summary({
    'Version': meson.project_version(),
    'Build type': get_option('config'),
    'C Compiler': cc.get_id() + ' ' + cc.version(),
    'C++ Compiler': cxx.get_id() + ' ' + cxx.version(),
    'Cross build': meson.is_cross_build(),
    'Image type': image_type,
    'Image filesystem': image_fs,
    'Image size': image_size,
    'QEMU available': qemu.found(),
    'GDB available': gdb.found(),
}, section: 'NeoOS Build Configuration')

message('Build configured successfully!')