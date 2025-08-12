message('Build stage2 (bootloader)...')

stage2_dir = meson.current_source_dir()
fs = import('fs')

toolchain_dir = join_paths(meson.project_source_root(),'toolchain', 'i686-elf', 'bin')
gcc_path = join_paths(toolchain_dir, 'i686-elf-gcc.exe')
gxx_path = join_paths(toolchain_dir, 'i686-elf-g++.exe')


if not fs.exists(gcc_path)
    error('Toolchain GCC not found at: ' + gcc_path)
endif

glob_cmd_c = ['python3', '-c',
'''

import glob
import sys
import json

path = sys.argv[1]
files = glob.glob(path + '/*.c')

print(json.dumps(files))

''', stage2_dir]

result = run_command(glob_cmd_c, check:true)
file_list = result.stdout().strip().replace('"','').replace('[', '').replace(']', '').replace(',', '').split()

stage2_c_sources = []
stage2_c_sources = files(file_list)

message('Found @0@ C source files for stage2'.format(stage2_c_sources.length().to_string()))


glob_cmd_asm = ['python3', '-c',
'''

import glob
import sys
import json

path = sys.argv[1]
files = glob.glob(path + '/*.asm')

print(json.dumps(files))

''', stage2_dir]

result_asm= run_command(glob_cmd_asm, check:true)
file_list_asm= result_asm.stdout().strip().replace('"','').replace('[', '').replace(']', '').replace(',', '').split()


message('Found @0@ ASM source files for stage2'.format(file_list_asm.length().to_string()))

stage2_asm_sources = []
foreach file : file_list_asm
    filename = file
    if filename != 'crti.asm' and filename != 'crtn.asm'
        stage2_asm_sources += files(file)
    endif
endforeach

crti_asm = files('crti.asm')[0]
crtn_asm = files('crtn.asm')[0]


if not fs.exists(crti_asm.full_path())
    error('crti.asm not found in stage2 directory')
endif


if not fs.exists(crtn_asm.full_path())
    error('crtn.asm not found in stage2 directory')
endif


crti_obj = custom_target(
    'stage2_crti_obj',
    input: crti_asm,
    output: 'crti.o',
    command: [
        nasm,
        '-f', 'elf',
        '-o', '@OUTPUT@',
        '@INPUT@'
    ],
    install: false
)


crtn_obj = custom_target(
    'stage2_crtn_obj',
    input: crtn_asm,
    output: 'crtn.o',
    command: [
        nasm,
        '-f', 'elf',
        '-o', '@OUTPUT@',
        '@INPUT@'
    ],
    install: false
)

stage2_asm_objects = []
foreach asm_file : stage2_asm_sources
    asm_name = fs.name(asm_file.full_path()).split('.')[0] + '_stage2'
    asm_obj = custom_target(
        'stage2_' + asm_name + '_obj',
        input: asm_file,
        output: asm_name + '.o',
        command: [
            nasm,
            '-f', 'elf',
            '-o', '@OUTPUT@',
            '@INPUT@'
        ],
        install: false
    )
    stage2_asm_objects += [asm_obj]
endforeach


toolchain_dir =  meson.project_source_root() / 'toolchain' / 'i686-elf'
libgcc_dir = toolchain_dir / 'lib' / 'gcc' / 'i686-elf' / '15.1.0'
toolchain_include_dir = libgcc_dir / 'include' 
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

stage2_linker_script = stage2_dir / 'linker.ld'

if not fs.exists(stage2_linker_script)
    error('Stage2 linker script not found: ' + stage2_linker_script)
endif

stage2_link_args = [
    '-T', stage2_linker_script,
    '-Map', meson.current_build_dir() / 'stage2.map',
    '-nostdlib',
    '-ffreestanding',
    '-static'
]

stage2_c_args = [
    '-std=c99',
    '-nostdlib',
    '-nodefaultlibs',
    '-fno-builtin',
    '-ffreestanding'
]


stage2_elf_sources = stage2_c_sources + stage2_asm_objects

stage2_bin = custom_target(
    'stage2.bin',
    input: stage2_elf_sources,
    output: 'stage2.bin',
    command: [
        gcc_path,
        '@INPUT@',
        stage2_c_args + ['-I' + toolchain_include_dir,  '-I.'] +
        [crti_obj] + (crtbegin_obj != '' ? [crtbegin_obj] : []) +
        (crtend_obj != '' ? [crtend_obj] : []) + [crtn_obj],
        '-o', '@OUTPUT@'
    ],
    install: false,
    build_by_default: true,
)