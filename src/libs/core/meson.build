# Core Library Build Configuration
# ================================
# This library provides core functionality shared between kernel and bootloader

message('Building libcore...')

# === Assembly Sources ===
libcore_asm_sources = files(
    'arch/i686/io_asm.asm',
)

# Create assembly objects using custom targets (NASM doesn't support multiple files)
libcore_asm_objects = []

foreach asm_file : libcore_asm_sources
    asm_name = asm_file.full_path().split('/')[-1].split('.')[0]
    asm_obj = custom_target(
        'libcore_' + asm_name + '_obj',
        input: asm_file,
        output: asm_name + '.o',
        command: [
            nasm,
            '-f', 'elf32',
            '-o', '@OUTPUT@',
            '@INPUT@'
        ],
        install: false,
    )
    libcore_asm_objects += [asm_obj]
endforeach

# === C++ Sources ===
libcore_cpp_sources = files(
    'arch/i686/E9Device.cpp',
    'arch/i686/IO.cpp',
    'dev/TextDevice.cpp',
)

# === Headers (for dependency tracking) ===
libcore_headers = files(
    'Defs.hpp',
    'FS/File.hpp',
    'arch/i686/E9Device.hpp',
    'arch/i686/IO.hpp',
    'arch/i686/VGATextDevice.hpp',
    'cpp/TypeTraits.hpp',
    'dev/BlockDevice.hpp',
    'dev/CharacterDevice.hpp',
    'dev/TextDevice.hpp',
)

# === Include Directories ===
libcore_inc = include_directories('.')

# === Build Static Library ===
libcore = static_library(
    'core',
    libcore_cpp_sources,
    libcore_asm_objects,
    libcore_headers,
    c_args: cflags,
    cpp_args: cxxflags,
    include_directories: libcore_inc,
    install: false,
    pic: false,  # Position independent code not needed for kernel
)

# Export library and include directory for other components
libcore_dep = declare_dependency(
    link_with: libcore,
    include_directories: libcore_inc,
)

message('libcore build configured successfully')