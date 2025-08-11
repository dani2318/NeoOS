#!/usr/bin/env python3
"""
NeoOS Build Automation Script
=============================

This script provides a convenient interface for building NeoOS with Meson.
It handles cross-compilation setup and provides common build tasks.

Usage:
    python3 build.py [command] [options]

Commands:
    setup       - Configure the build system
    build       - Build the project
    clean       - Clean build files
    rebuild     - Clean and build
    run         - Build and run in QEMU
    debug       - Build and run with debugger
    help        - Show this help message

Options:
    --config=debug|release     Build configuration (default: debug)
    --image-type=hdd|floppy    Image type (default: hdd)
    --image-fs=fat12|fat16|fat32  Filesystem (default: fat32)
    --toolchain=auto|i686-elf|gcc    Toolchain to use (default: auto)
    --verbose                  Enable verbose output
    --jobs=N                   Number of build jobs (default: auto)

Examples:
    python3 build.py setup
    python3 build.py build --config=release
    python3 build.py run --image-type=floppy
    python3 build.py rebuild --config=release --verbose
"""

import sys
import os
import subprocess
import shutil
import argparse
from pathlib import Path

class Colors:
    """ANSI color codes for terminal output"""
    RED = '\033[0;31m'
    GREEN = '\033[0;32m'
    YELLOW = '\033[0;33m'
    BLUE = '\033[0;34m'
    PURPLE = '\033[0;35m'
    CYAN = '\033[0;36m'
    WHITE = '\033[0;37m'
    BOLD = '\033[1m'
    NC = '\033[0m'  # No Color

def print_status(message, color=Colors.GREEN):
    """Print a colored status message"""
    print(f"{color}[INFO]{Colors.NC} {message}")

def print_error(message):
    """Print an error message"""
    print(f"{Colors.RED}[ERROR]{Colors.NC} {message}", file=sys.stderr)

def print_warning(message):
    """Print a warning message"""
    print(f"{Colors.YELLOW}[WARNING]{Colors.NC} {message}")

def run_command(cmd, check=True, cwd=None, capture_output=False):
    """Run a command and handle errors"""
    if isinstance(cmd, list):
        cmd_str = ' '.join(cmd)
    else:
        cmd_str = cmd
        cmd = cmd.split()
    
    print_status(f"Running: {cmd_str}")
    
    try:
        result = subprocess.run(
            cmd, 
            check=check, 
            cwd=cwd,
            capture_output=capture_output,
            text=True
        )
        return result
    except subprocess.CalledProcessError as e:
        print_error(f"Command failed: {cmd_str}")
        print_error(f"Exit code: {e.returncode}")
        if e.stdout:
            print_error(f"Stdout: {e.stdout}")
        if e.stderr:
            print_error(f"Stderr: {e.stderr}")
        sys.exit(e.returncode)
    except FileNotFoundError:
        print_error(f"Command not found: {cmd[0]}")
        sys.exit(1)

def check_dependencies():
    """Check for required build dependencies"""
    dependencies = {
        'meson': 'Build system',
        'ninja': 'Build backend', 
        'python3': 'Python interpreter',
        'nasm': 'Netwide Assembler',
    }
    
    optional_deps = {
        'i686-elf-gcc': 'i686 cross-compiler (recommended)',
        'gcc': 'System GCC (fallback)',
        'qemu-system-i386': 'QEMU emulator for testing',
        'gdb': 'GNU debugger',
    }
    
    print_status("Checking dependencies...")
    
    missing_required = []
    for dep, desc in dependencies.items():
        if not shutil.which(dep):
            missing_required.append((dep, desc))
    
    if missing_required:
        print_error("Missing required dependencies:")
        for dep, desc in missing_required:
            print(f"  - {dep}: {desc}")
        return False
    
    missing_optional = []
    for dep, desc in optional_deps.items():
        if not shutil.which(dep):
            missing_optional.append((dep, desc))
    
    if missing_optional:
        print_warning("Missing optional dependencies:")
        for dep, desc in missing_optional:
            print(f"  - {dep}: {desc}")
    
    print_status("Dependencies OK")
    return True

def detect_toolchain():
    """Auto-detect the best available toolchain"""
    if shutil.which('i686-elf-gcc'):
        print_status("Using i686-elf cross-compiler toolchain")
        return 'i686-elf'
    elif shutil.which('gcc'):
        print_warning("Using system GCC (not optimal for OS development)")
        return 'gcc'
    else:
        print_error("No suitable compiler found")
        return None

def create_cross_file(toolchain):
    """Create appropriate cross-compilation file"""
    cross_file = Path('build-cross.ini')
    
    if toolchain == 'i686-elf':
        content = """
[binaries]
c = 'i686-elf-gcc'
cpp = 'i686-elf-g++'
ar = 'i686-elf-ar'
ld = 'i686-elf-ld'
objcopy = 'i686-elf-objcopy'
strip = 'i686-elf-strip'
size = 'i686-elf-size'
nm = 'i686-elf-nm'
objdump = 'i686-elf-objdump'
nasm = 'nasm'
python3 = 'python3'

[properties]
needs_exe_wrapper = false
skip_sanity_check = true

[host_machine]
system = 'none'
cpu_family = 'x86'
cpu = 'i686'
endian = 'little'
"""
    else:  # gcc fallback
        content = """
[binaries]
c = 'gcc'
cpp = 'g++'
ar = 'ar'
ld = 'ld'
objcopy = 'objcopy'
strip = 'strip'
size = 'size'
nm = 'nm'
objdump = 'objdump'
nasm = 'nasm'
python3 = 'python3'

[properties]
needs_exe_wrapper = false
skip_sanity_check = true

[host_machine]
system = 'none'
cpu_family = 'x86'
cpu = 'i686'
endian = 'little'
"""
    
    with open(cross_file, 'w') as f:
        f.write(content.strip())
    
    return cross_file

def setup_build(args):
    """Configure the build system"""
    print_status("Setting up NeoOS build system...")
    
    if not check_dependencies():
        sys.exit(1)
    
    # Detect toolchain
    if args.toolchain == 'auto':
        toolchain = detect_toolchain()
        if not toolchain:
            sys.exit(1)
    else:
        toolchain = args.toolchain
    
    # Create cross-compilation file
    cross_file = create_cross_file(toolchain)
    
    # Build directory
    build_dir = Path('build')
    if build_dir.exists() and args.reconfigure:
        print_status("Removing existing build directory...")
        shutil.rmtree(build_dir)
    
    # Meson setup command
    cmd = [
        'meson', 'setup', str(build_dir),
        '--cross-file', str(cross_file),
        f'-Dconfig={args.config}',
        f'-DimageType={args.image_type}',
        f'-DimageFS={args.image_fs}',
    ]
    
    if args.verbose:
        cmd.append('--verbose')
    
    run_command(cmd)
    print_status("Build system configured successfully!")

def build_project(args):
    """Build the project"""
    build_dir = Path('build')
    
    if not build_dir.exists():
        print_error("Build not configured. Run 'python3 build.py setup' first.")
        sys.exit(1)
    
    print_status("Building NeoOS...")
    
    cmd = ['meson', 'compile', '-C', str(build_dir)]
    
    if args.jobs:
        cmd.extend(['-j', str(args.jobs)])
    
    if args.verbose:
        cmd.append('--verbose')
    
    run_command(cmd)
    print_status("Build completed successfully!")

def clean_build(args):
    """Clean build files"""
    build_dir = Path('build')
    
    if build_dir.exists():
        print_status("Cleaning build directory...")
        shutil.rmtree(build_dir)
        print_status("Clean completed!")
    else:
        print_status("Nothing to clean")

def run_project(args):
    """Build and run the project"""
    build_project(args)
    
    build_dir = Path('build')
    print_status("Running NeoOS in QEMU...")
    
    # Look for the run target
    cmd = ['meson', 'compile', '-C', str(build_dir), 'run']
    run_command(cmd)

def debug_project(args):
    """Build and run with debugger"""
    build_project(args)
    
    build_dir = Path('build')
    print_status("Starting NeoOS with debugger...")
    
    # Look for the debug target
    cmd = ['meson', 'compile', '-C', str(build_dir), 'debug']
    run_command(cmd)

def main():
    parser = argparse.ArgumentParser(
        description='NeoOS Build System',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__
    )
    
    parser.add_argument(
        'command',
        choices=['setup', 'build', 'clean', 'rebuild', 'run', 'debug', 'help'],
        help='Build command to execute'
    )
    
    parser.add_argument(
        '--config',
        choices=['debug', 'release'],
        default='debug',
        help='Build configuration'
    )
    
    parser.add_argument(
        '--image-type',
        choices=['hdd', 'floppy'],
        default='hdd',
        help='Disk image type'
    )
    
    parser.add_argument(
        '--image-fs',
        choices=['fat12', 'fat16', 'fat32'],
        default='fat32',
        help='Filesystem type'
    )
    
    parser.add_argument(
        '--toolchain',
        choices=['auto', 'i686-elf', 'gcc'],
        default='auto',
        help='Toolchain to use'
    )
    
    parser.add_argument(
        '--verbose',
        action='store_true',
        help='Enable verbose output'
    )
    
    parser.add_argument(
        '--jobs',
        type=int,
        help='Number of parallel jobs'
    )
    
    parser.add_argument(
        '--reconfigure',
        action='store_true',
        help='Force reconfiguration'
    )
    
    args = parser.parse_args()
    
    if args.command == 'help':
        parser.print_help()
        return
    
    # Execute command
    try:
        if args.command == 'setup':
            setup_build(args)
        elif args.command == 'build':
            build_project(args)
        elif args.command == 'clean':
            clean_build(args)
        elif args.command == 'rebuild':
            clean_build(args)
            setup_build(args)
            build_project(args)
        elif args.command == 'run':
            run_project(args)
        elif args.command == 'debug':
            debug_project(args)
            
    except KeyboardInterrupt:
        print_error("Build interrupted by user")
        sys.exit(1)

if __name__ == '__main__':
    main()