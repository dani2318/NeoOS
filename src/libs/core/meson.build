message('Building libcore...')

libcore_asm_sources = files(
  'arch/i686/io_asm.asm',
)

libcore_asm_objects = []

foreach asm_file : libcore_asm_sources
  asm_path = '@0@'.format(asm_file)
  asm_name = asm_path.split('/')[-1].split('.')[0]
  asm_obj = custom_target(
    'libcore_' + asm_name + '_obj',
    input: asm_file,
    output: asm_name + '.o',
    command: [
      nasm,
      '-f', 'elf32',
      '-o', '@OUTPUT@',
      '@INPUT@',
    ],
    install: false,
  )
  libcore_asm_objects += [asm_obj]
endforeach

libcore_cpp_sources = files(
  'arch/i686/E9Device.cpp',
  'arch/i686/IO.cpp',
  'dev/TextDevice.cpp',
)

# Use project toolchain include directories
toolchain_base = meson.project_source_root() / 'toolchain' / 'i686-elf'
toolchain_include = toolchain_base / 'lib' / 'gcc' / 'i686-elf' / '15.1.0' / 'include'

libcore_inc = [include_directories('.')]

libcore = static_library(
  'core',
  libcore_cpp_sources,
  libcore_asm_objects,
  c_args: cflags + ['-I' + toolchain_include, '-I.'],
  cpp_args: cxxflags + ['-I' + toolchain_include, '-I.'],
  include_directories: libcore_inc,
  install: false,
  pic: false,
)

libcore_dep = declare_dependency(
  link_with: libcore,
  include_directories: libcore_inc,
)

message('libcore build configured successfully')