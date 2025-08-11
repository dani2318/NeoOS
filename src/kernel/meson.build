# Kernel Build Configuration
# ==========================
# The main NeoOS kernel

message('Building kernel...')

# === Configuration ===
kernel_dir = meson.current_source_dir()

# === Auto-discover source files ===
fs = import('fs')

# Recursively find all C source files
kernel_c_sources = []
kernel_c_dirs = [
    'arch/generic',
    'arch/i686',
    'arch/i686/interrupts', 
    'arch/i686/pic',
    'hal',
    '.',  # Current directory
]

foreach dir : kernel_c_dirs
    dir_path = kernel_dir / dir
    if fs.is_dir(dir_path)
        foreach file : fs.glob(dir_path, '*.c')
            kernel_c_sources += files(file)
        endforeach
    endif
endforeach

# Remove any files we don't want to compile directly
excluded_files = []  # Add any files to exclude here if needed

final_c_sources = []
foreach src : kernel_c_sources
    include_file = true
    foreach excluded : excluded_files
        if src.full_path().contains(excluded)
            include_file = false
            break
        endif
    endforeach
    if include_file
        final_c_sources += [src]
    endif
endforeach

message('Found @0@ C source files for kernel'.format(final_c_sources.length()))

# Auto-discover assembly sources (excluding CRT files)
kernel_asm_sources = []
kernel_asm_dirs = [
    'arch/i686',
    'arch/i686/interrupts',
    '.',
]

foreach dir : kernel_asm_dirs
    dir_path = kernel_dir / dir
    if fs.is_dir(dir_path)
        foreach file : fs.glob(dir_path, '*.asm')
            # Skip CRT files - they need special handling
            if not (file.contains('crti.asm') or file.contains('crtn.asm'))
                kernel_asm_sources += files(file)
            endif
        endforeach
    endif
endforeach

message('Found @0@ assembly source files for kernel'.format(kernel_asm_sources.length()))

# === CRT Files (Critical ordering) ===
crti_asm = files('crti.asm')[0]
crtn_asm = files('crtn.asm')[0]

# Verify CRT files exist
if not fs.exists(crti_asm)
    error('crti.asm not found in kernel directory')
endif
if not fs.exists(crtn_asm)
    error('crtn.asm not found in kernel directory')
endif

# === Generated Files ===
# Generate ISR include file needed by interrupt handling
isr_generator_script = meson.project_source_root() / 'scripts' / 'generate_isr.py'

if not fs.exists(isr_generator_script)
    error('ISR generator script not found: ' + isr_generator_script)
endif

isrs_gen_inc = custom_target(
    'isrs_gen.inc',
    input: isr_generator_script,
    output: 'isrs_gen.inc',
    command: [python3, '@INPUT@', '-o', '@OUTPUT@'],
    console: true,
    build_by_default: true,
)

# === Build Assembly Objects ===
# Build CRT objects first
crti_obj = custom_target(
    'kernel_crti_obj',
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
    'kernel_crtn_obj', 
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

# Build other assembly objects
kernel_asm_objects = []
foreach asm_file : kernel_asm_sources
    asm_name = fs.name(asm_file.full_path()).split('.')[0] + '_kernel'
    asm_obj = custom_target(
        'kernel_' + asm_name + '_obj',
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
    kernel_asm_objects += [asm_obj]
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
        message('Found libgcc CRT objects in: ' + libgcc_dir)
    endif
endif

# Method 2: Common locations fallback
if crtbegin_obj == ''
    common_libgcc_paths = [
        '/usr/lib/gcc/i686-linux-gnu/*/32',
        '/usr/lib/gcc/*/32', 
        '/usr/local/lib/gcc/i686-elf/*/32',
        '/opt/cross/lib/gcc/i686-elf/*/32',
    ]
    
    foreach pattern : common_libgcc_paths
        # Note: Meson doesn't have great glob support, so this is simplified
        # In a real implementation, you might need a Python script
        if fs.exists('/usr/lib/gcc')
            # Try some common specific paths
            test_dirs = [
                '/usr/lib/gcc/x86_64-linux-gnu/11/32',
                '/usr/lib/gcc/x86_64-linux-gnu/12/32', 
                '/usr/lib/gcc/x86_64-linux-gnu/13/32',
            ]
            
            foreach test_dir : test_dirs
                potential_crtbegin = test_dir / 'crtbegin.o'
                potential_crtend = test_dir / 'crtend.o'
                
                if fs.exists(potential_crtbegin) and fs.exists(potential_crtend)
                    crtbegin_obj = potential_crtbegin
                    crtend_obj = potential_crtend
                    libgcc_dir = test_dir
                    message('Found libgcc CRT objects in: ' + libgcc_dir)
                    break
                endif
            endforeach
            
            if crtbegin_obj != ''
                break
            endif
        endif
    endforeach
endif

# If still not found, create a script to find them
if crtbegin_obj == ''
    message('Searching for libgcc CRT objects...')
    
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
                error('Failed to find libgcc CRT objects: ' + line.split('=')[1])
            endif
        endforeach
    endif
endif

if crtbegin_obj == ''
    warning('libgcc CRT objects not found - linking may fail')
    warning('You may need to install gcc-multilib or specify libgcc path manually')
else
    message('Using CRT objects:')
    message('  crtbegin.o: ' + crtbegin_obj)
    message('  crtend.o: ' + crtend_obj)
endif

# === Linker Configuration ===
kernel_linker_script = kernel_dir / 'linker.ld'

# Verify linker script exists
if not fs.exists(kernel_linker_script)
    error('Kernel linker script not found: ' + kernel_linker_script)
endif

kernel_link_args = [
    '-T', kernel_linker_script,
    '-Map', meson.current_build_dir() / 'kernel.map',
    '-nostdlib',
    '-static',
    '--gc-sections',  # Remove unused sections for smaller kernel
    '--warn-common',  # Warn about common symbols
]

# === Kernel-specific Compile Flags ===
kernel_c_args = cflags + [
    '-fno-pie',          # Disable position independent executable
    '-static',           # Static linking
    '-fno-omit-frame-pointer',  # Keep frame pointers for debugging
    '-mno-red-zone',     # Don't use red zone (x86_64 feature, but good practice)
]

# Add debug-specific flags
if get_option('config') == 'debug'
    kernel_c_args += [
        '-fno-inline',   # Don't inline functions for better debugging
        '-ggdb3',        # Maximum debug information
    ]
endif

# === Build Kernel ELF with Proper CRT Ordering ===
# We need to ensure the correct linking order:
# crti.o -> crtbegin.o -> objects -> libraries -> crtend.o -> crtn.o

kernel_objects_and_libs = []

# Add regular C objects (will be built automatically by executable())
# Add regular assembly objects
kernel_objects_and_libs += kernel_asm_objects
kernel_objects_and_libs += [isrs_gen_inc]  # Include generated ISR file

# Create a custom link command that respects CRT ordering
if crtbegin_obj != ''
    # Create a response file with the correct order
    kernel_link_order = custom_target(
        'kernel_link_order.rsp',
        output: 'kernel_link_order.rsp',
        command: [
            python3, '-c', 
            '''
import sys
with open(sys.argv[1], 'w') as f:
    f.write('@0@\\n'.format("@1@"))  # crtbegin.o
    f.write('@0@\\n'.format("@2@"))  # crtend.o
            '''.format(sys.argv[1], crtbegin_obj, crtend_obj),
            '@OUTPUT@'
        ]
    )
    
    kernel_extra_files = [kernel_link_order]
else
    kernel_extra_files = []
endif

# Build the kernel executable
kernel_elf_sources = final_c_sources + kernel_objects_and_libs

kernel_elf = executable(
    'kernel.elf',
    kernel_elf_sources,
    objects: [crti_obj] + (crtbegin_obj != '' ? [crtbegin_obj] : []) + 
             (crtend_obj != '' ? [crtend_obj] : []) + [crtn_obj],
    c_args: kernel_c_args,
    link_args: kernel_link_args,
    dependencies: [libcore_dep],
    install: false,
    native: false,  # This is a cross-compiled target
)

# === Create Stripped Kernel (for release) ===
kernel_stripped_elf = custom_target(
    'kernel-stripped.elf',
    input: kernel_elf,
    output: 'kernel-stripped.elf', 
    command: [
        strip_prog,
        '--strip-debug',
        '--strip-unneeded',
        '-o', '@OUTPUT@',
        '@INPUT@'
    ],
    install: false,
)

# === Optional Debug Outputs ===
if get_option('config') == 'debug'
    # Generate kernel disassembly
    objdump = find_program('objdump', required: false)
    if objdump.found()
        kernel_disasm = custom_target(
            'kernel.dis',
            input: kernel_elf,
            output: 'kernel.dis',
            command: [
                objdump,
                '-d', '-S', '-l',  # Disassemble with source and line numbers
                '@INPUT@'
            ],
            capture: true,
            install: false,
        )
    endif
    
    # Generate symbol map
    nm = find_program('nm', required: false)
    if nm.found()
        kernel_symbols = custom_target(
            'kernel.sym',
            input: kernel_elf,
            output: 'kernel.sym', 
            command: [
                nm,
                '--numeric-sort',
                '--print-size',
                '@INPUT@'
            ],
            capture: true,
            install: false,
        )
    endif
endif

# === Size Information ===
kernel_size_info = custom_target(
    'kernel_size_info',
    input: kernel_elf,
    output: 'kernel_size.txt',
    command: [
        python3, '-c',
        '''
import sys, os, subprocess
try:
    # Get ELF file size
    size = os.path.getsize(sys.argv[1])
    
    # Try to get section sizes with size command
    try:
        result = subprocess.run(["size", sys.argv[1]], capture_output=True, text=True)
        if result.returncode == 0:
            size_output = result.stdout.strip()
        else:
            size_output = "Size command failed"
    except:
        size_output = "Size command not available"
    
    print(f"Kernel ELF size: {size} bytes ({size/1024:.1f} KB)")
    print("Section sizes:")
    print(size_output)
    
    with open(sys.argv[2], 'w') as f:
        f.write(f"Kernel size: {size} bytes\\n")
        f.write(size_output + "\\n")
        
except Exception as e:
    print(f"Error getting kernel size: {e}")
    with open(sys.argv[2], 'w') as f:
        f.write("Size calculation failed\\n")
        ''',
        '@INPUT@', '@OUTPUT@'
    ],
    depends: [kernel_elf],
    console: true,
    build_by_default: true,
)

message('Kernel build configured successfully')