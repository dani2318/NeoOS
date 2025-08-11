# Stage 2 Bootloader Build Configuration
# =======================================
# The second stage bootloader - loads and starts the kernel

message('Building stage2 bootloader...')

# === Configuration ===
stage2_dir = meson.current_source_dir()
fs = import('fs')

# === Auto-discover C source files ===
stage2_c_sources = []
foreach file : fs.glob(stage2_dir, '*.c')
    stage2_c_sources += files(file)
endforeach

message('Found @0@ C source files for stage2'.format(stage2_c_sources.length()))

# Verify we found some sources
if stage2_c_sources.length() == 0
    error('No C source files found in stage2 directory')
endif

# === Auto-discover assembly sources (excluding CRT files) ===
stage2_asm_sources = []
foreach file : fs.glob(stage2_dir, '*.asm')
    # Skip CRT files - they need special handling
    if not (file.contains('crti.asm') or file.contains('crtn.asm'))
        stage2_asm_sources += files(file)
    endif
endforeach

message('Found @0@ assembly source files for stage2'.format(stage2_asm_sources.length()))

# === CRT Files (Critical ordering) ===
crti_asm = files('crti.asm')[0]
crtn_asm = files('crtn.asm')[0]

# Verify CRT files exist
if not fs.exists(crti_asm)
    error('crti.asm not found in stage2 directory')
endif
if not fs.exists(crtn_asm)
    error('crtn.asm not found in stage2 directory')
endif

# === Build CRT Assembly Objects ===
crti_obj = custom_target(
    'stage2_crti_obj',
    input: crti_asm,
    output: 'crti.o',
    command: [
        nasm,
        '-f', 'elf32',
        '-o', '@OUTPUT@',
        '@INPUT@'
    ],
    install: false,
)

crtn_obj = custom_target(
    'stage2_crtn_obj',
    input: crtn_asm,
    output: 'crtn.o',
    command: [
        nasm,
        '-f', 'elf32',
        '-o', '@OUTPUT@',
        '@INPUT@'
    ],
    install: false,
)

# === Build Other Assembly Objects ===
stage2_asm_objects = []
foreach asm_file : stage2_asm_sources
    asm_name = fs.name(asm_file.full_path()).split('.')[0] + '_stage2'
    asm_obj = custom_target(
        'stage2_' + asm_name + '_obj',
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
    stage2_asm_objects += [asm_obj]
endforeach

# === Detect libgcc for CRT objects ===
cc = meson.get_compiler('c')

# Try to find libgcc directory containing crtbegin.o and crtend.o
libgcc_dir = ''
crtbegin_obj = ''
crtend_obj = ''

# Method 1: Ask GCC directly
gcc_libdir_result = run_command(
    cc.get_id() == 'gcc' ? cc.cmd_array()[0] : 'gcc',
    '-m32', '--print-libgcc-file-name',
    capture: true,
    check: false
)

if gcc_libdir_result.returncode() == 0
    libgcc_file = gcc_libdir_result.stdout().strip()
    libgcc_dir = fs.parent(libgcc_file)
    
    potential_crtbegin = libgcc_dir / 'crtbegin.o'
    potential_crtend = libgcc_dir / 'crtend.o'
    
    if fs.exists(potential_crtbegin) and fs.exists(potential_crtend)
        crtbegin_obj = potential_crtbegin
        crtend_obj = potential_crtend
        message('Found libgcc CRT objects for stage2 in: ' + libgcc_dir)
    endif
endif

# If not found, use the same search logic as kernel
if crtbegin_obj == ''
    message('Searching for libgcc CRT objects for stage2...')
    
    find_libgcc_script = '''
import subprocess
import os
import glob

def find_crt_objects():
    try:
        # Ask GCC for libgcc location
        result = subprocess.run([@0@, '-m32', '--print-libgcc-file-name'], 
                              capture_output=True, text=True, check=True)
        libgcc_file = result.stdout.strip()
        libgcc_dir = os.path.dirname(libgcc_file)
        
        crtbegin = os.path.join(libgcc_dir, 'crtbegin.o')
        crtend = os.path.join(libgcc_dir, 'crtend.o')
        
        if os.path.exists(crtbegin) and os.path.exists(crtend):
            print(f"LIBGCC_DIR={libgcc_dir}")
            print(f"CRTBEGIN={crtbegin}")
            print(f"CRTEND={crtend}")
            return
            
        # Fallback: search common locations
        search_paths = [
            "/usr/lib/gcc/*/*/32/crtbegin.o",
            "/usr/local/lib/gcc/*/*/32/crtbegin.o", 
            "/opt/cross/lib/gcc/*/*/32/crtbegin.o",
        ]
        
        for pattern in search_paths:
            matches = glob.glob(pattern)
            if matches:
                crtbegin = matches[0]
                crtend = crtbegin.replace('crtbegin.o', 'crtend.o')
                if os.path.exists(crtend):
                    libgcc_dir = os.path.dirname(crtbegin)
                    print(f"LIBGCC_DIR={libgcc_dir}")
                    print(f"CRTBEGIN={crtbegin}")
                    print(f"CRTEND={crtend}")
                    return
                    
        print("ERROR=CRT objects not found")
        
    except Exception as e:
        print(f"ERROR={e}")

if __name__ == "__main__":
    find_crt_objects()
    '''.format("'" + (cc.get_id() == 'gcc' ? cc.cmd_array()[0] : 'gcc') + "'")
    
    libgcc_result = run_command(
        python3, '-c', find_libgcc_script,
        capture: true,
        check: false
    )
    
    if libgcc_result.returncode() == 0
        lines = libgcc_result.stdout().strip().split('\n')
        foreach line : lines
            if line.startswith('LIBGCC_DIR=')
                libgcc_dir = line.split('=')[1]
            elif line.startswith('CRTBEGIN=')
                crtbegin_obj = line.split('=')[1]
            elif line.startswith('CRTEND=')
                crtend_obj = line.split('=')[1]
            elif line.startswith('ERROR=')
                warning('Failed to find libgcc CRT objects for stage2: ' + line.split('=')[1])
            endif
        endforeach
    endif
endif

if crtbegin_obj == ''
    warning('libgcc CRT objects not found for stage2 - linking may fail')
    warning('You may need to install gcc-multilib or specify libgcc path manually')
else
    message('Using CRT objects for stage2:')
    message('  crtbegin.o: ' + crtbegin_obj)
    message('  crtend.o: ' + crtend_obj)
endif

# === Linker Configuration ===
stage2_linker_script = stage2_dir / 'linker.ld'

# Verify linker script exists
if not fs.exists(stage2_linker_script)
    error('Stage2 linker script not found: ' + stage2_linker_script)
endif

stage2_link_args = [
    '-T', stage2_linker_script,
    '-Map', meson.current_build_dir() / 'stage2.map',
    '-nostdlib',
    '-static',
    '--gc-sections',  # Remove unused sections
]

# === Stage2-specific Compile Flags ===
stage2_c_args = cflags + [
    '-fno-pie',          # Disable position independent executable
    '-static',           # Static linking
]

# === Build Stage2 ELF with Proper CRT Ordering ===
# Correct linking order: crti.o -> crtbegin.o -> objects -> libraries -> crtend.o -> crtn.o

stage2_objects_and_libs = stage2_asm_objects

# Build the stage2 executable with proper CRT ordering
stage2_elf_sources = stage2_c_sources + stage2_objects_and_libs

stage2_elf = executable(
    'stage2.elf',
    stage2_elf_sources,
    objects: [crti_obj] + (crtbegin_obj != '' ? [crtbegin_obj] : []) + 
             (crtend_obj != '' ? [crtend_obj] : []) + [crtn_obj],
    c_args: stage2_c_args,
    link_args: stage2_link_args,
    dependencies: [libcore_dep],
    install: false,
    native: false,  # This is a cross-compiled target
)

# === Create Raw Binary ===
stage2_bin = custom_target(
    'stage2.bin',
    input: stage2_elf,
    output: 'stage2.bin',
    command: [
        objcopy,
        '-O', 'binary',
        '@INPUT@',
        '@OUTPUT@'
    ],
    install: false,
    build_by_default: true,
)

# === Optional: Create Disassembly ===
if get_option('config') == 'debug'
    objdump = find_program('objdump', required: false)
    if objdump.found()
        stage2_disasm = custom_target(
            'stage2.dis',
            input: stage2_elf,
            output: 'stage2.dis',
            command: [
                objdump,
                '-d', '-S',
                '@INPUT@'
            ],
            capture: true,
            install: false,
        )
    endif
endif

# === Size Information ===
stage2_size_info = custom_target(
    'stage2_size_info',
    input: stage2_bin,
    output: 'stage2_size.txt',
    command: [
        python3, '-c',
        '''
import sys, os
size = os.path.getsize(sys.argv[1])
print(f"Stage2 bootloader size: {size} bytes ({size/1024:.1f} KB)")
with open(sys.argv[2], 'w') as f:
    f.write(f"Stage2 size: {size} bytes\\n")
    ''',
        '@INPUT@', '@OUTPUT@'
    ],
    depends: [stage2_bin],
    console: true,
    build_by_default: true,
)

message('Stage2 bootloader build configured successfully')