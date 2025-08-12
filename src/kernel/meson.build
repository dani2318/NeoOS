
message('Building kernel...')

isr_generator_script = join_paths(meson.project_source_root(), '/scripts/generate_isr.py')

isrs_gen_inc = custom_target(
    'isrs_gen.inc',
    input: '../../scripts/generate_isr.py',
    output: 'isrs_gen.inc',
    command: [python3, '@INPUT@','-o','@OUTPUT@'],
    console: true,
    build_by_default: true,
)

kernel_dir = meson.current_source_dir()
fs = import('fs')

sources = run_command('python3', meson.project_source_root() + '/scripts/list_C_files.py', kernel_dir, capture:true,check: true,)
kernel_c_sources =  sources.stdout().strip().split('\n')
message('Found @0@ C sources files for kernel'.format(kernel_c_sources.length()))

sources_asm = run_command('python3', meson.project_source_root() + '/scripts/list_ASM_files.py', kernel_dir, capture:true,check: true,)
kernel_asm_sources = sources_asm.stdout().strip().split('\n')
message('Found @0@ ASM sources files for kernel'.format(kernel_asm_sources.length()))

crti_asm = files('crti.asm')[0]
crtn_asm = files('crtn.asm')[0]

if not fs.exists(crti_asm.full_path())
    error('crti.asm not found in kernel directory')
endif

if not fs.exists(crtn_asm.full_path())
    error('crtn.asm not found in kernel directory')
endif

crti_obj = custom_target(
    'kernel_crti_obj',
    input: crti_asm,
    output: 'crti.o',
    command: [nasm, '-f','elf', '-o','@OUTPUT@','@INPUT@'],
    install: false,
)

crtn_obj = custom_target(
    'kernel_crtn_obj',
    input: crtn_asm,
    output: 'crtn.o',
    command: [nasm, '-f','elf', '-o','@OUTPUT@','@INPUT@'],
    install: false,
)

kernel_asm_objects = []
foreach asm_file : kernel_asm_sources
    asm_name = fs.name(asm_file).split('.')[0] + '_kernel'
    asm_obj = custom_target(
        'kernel_' + asm_name + '_obj',
        input: asm_file,
        output: asm_name + '.o',
        command: [
            nasm,
            '-I', meson.project_source_root() + '/src/kernel/arch/i686',
            '-f', 'elf',
            '-o', '@OUTPUT@',
            '@INPUT@',
        ],
        install: false,
    )

    kernel_asm_objects += [asm_obj]
endforeach

toolchain_base = meson.project_source_root() / 'toolchain' / 'i686-elf'
libgcc_dir = toolchain_base / 'lib' / 'gcc' / 'i686-elf' / '15.1.0'
toolchain_include = libgcc_dir / 'include'

crtbegin_obj = libgcc_dir / 'crtbegin.o'
crtend_obj = libgcc_dir / 'crtend.o'

if fs.exists(crtbegin_obj) and fs.exists(crtend_obj)
    message('Found toolchain CRT Objects:')
    message('   crtbegin.o: ' + crtbegin_obj)
    message('   crtend.o: ' + crtend_obj)
else
    message('toolchain CRT Objects NOT found:')
    message('Expected at: ' + libgcc_dir)
    crtbegin_obj = ''
    crtend_obj = ''
endif

kernel_linker_script = kernel_dir / 'linker.ld'

if not fs.exists(kernel_linker_script)
    error('Kernel linker script not found: ' + kernel_linker_script)
endif

kernel_link_args = [
    '-T', kernel_linker_script,
    '-Map', meson.current_build_dir() / 'kernel.map',
    '-nostdlib',
    '-static',
]

kernel_c_args = [
    '-fno-pie',
    '-static',
    '-mno-red-zone',
]

kernel_objects_and_libs = kernel_asm_objects + [isrs_gen_inc]

kernel_elf_sources = kernel_c_sources + kernel_objects_and_libs

kernel_elf = executable(
    'kernel',
    kernel_elf_sources,
    objects: [crti_obj] + (crtbegin_obj != '' ? [crtbegin_obj ] : []) + (crtend_obj != '' ? [crtend_obj ] : []) + [crtn_obj],
    c_args: ['-I' + toolchain_include, '-I.'],
    cpp_args: ['-I' + toolchain_include, '-I.'],
    link_args: kernel_link_args,
    dependencies: [libcore_dep],
    install: false,
    native: false,
    name_suffix: 'elf',
)

kernel_elf = custom_target(
    'kernel-stripped.elf',
    input: kernel_elf,
    output: 'kernel-stripped.elf',
    command: [
        strip_prog,
        '--strip-debug',
        '--strip-unneeded',
        '-o', '@OUTPUT@',
        '@INPUT@',
    ],
    install: false,
)

message('Kernel build configured successfully!')