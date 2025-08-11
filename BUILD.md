# NeoOS Build System

This document explains how to build NeoOS using the improved Meson build system.

## Quick Start

```bash
# 1. Install dependencies (see below)
# 2. Configure the build
python3 build.py setup

# 3. Build the OS
python3 build.py build

# 4. Run in QEMU
python3 build.py run
```

## Dependencies

### Required Tools
- **Meson** (≥0.50.0) - Build system
- **Ninja** - Build backend 
- **Python 3** - Build scripts
- **NASM** - Netwide Assembler for assembly code

### Compiler Options

**Option 1: i686-elf Cross-Compiler (Recommended)**
```bash
# Download from: https://github.com/lordmilko/i686-elf-tools
# Or build your own cross-compiler

# Windows (MSYS2):
pacman -S mingw-w64-i686-gcc

# Linux:
# Install i686-elf-gcc from your package manager or build from source
```

**Option 2: System GCC (Fallback)**
```bash
# Windows (MSYS2):
pacman -S mingw-w64-x86_64-gcc

# Linux:
sudo apt install gcc-multilib  # Ubuntu/Debian
sudo dnf install gcc           # Fedora
```

### Optional Tools
- **QEMU** - For running/testing the OS
- **GDB** - For debugging

### Installation Examples

**Windows (MSYS2):**
```bash
# Install MSYS2 from https://www.msys2.org/
pacman -S meson ninja python3 nasm
pacman -S mingw-w64-x86_64-gcc  # or mingw-w64-i686-gcc
pacman -S qemu gdb  # optional
```

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install meson ninja-build python3 nasm gcc-multilib
sudo apt install qemu-system-x86 gdb  # optional
```

**Fedora:**
```bash
sudo dnf install meson ninja-build python3 nasm gcc
sudo dnf install qemu-system-x86 gdb  # optional
```

**macOS:**
```bash
brew install meson ninja python3 nasm
brew install qemu gdb  # optional
# You'll need to build or install i686-elf-gcc separately
```

## Build Commands

### Using the Build Script (Recommended)

```bash
# Setup build system
python3 build.py setup [options]

# Build project
python3 build.py build [options]

# Clean build files
python3 build.py clean

# Clean and rebuild
python3 build.py rebuild [options]

# Build and run in QEMU
python3 build.py run [options]

# Build and debug with GDB
python3 build.py debug [options]

# Show help
python3 build.py help
```

#### Build Options

- `--config=debug|release` - Build configuration (default: debug)
- `--image-type=hdd|floppy` - Disk image type (default: hdd)  
- `--image-fs=fat12|fat16|fat32` - Filesystem (default: fat32)
- `--toolchain=auto|i686-elf|gcc` - Compiler toolchain (default: auto)
- `--verbose` - Enable verbose output
- `--jobs=N` - Number of parallel build jobs

#### Examples

```bash
# Debug build (default)
python3 build.py setup
python3 build.py build

# Release build with optimizations
python3 build.py setup --config=release
python3 build.py build

# Build floppy disk image
python3 build.py setup --image-type=floppy
python3 build.py run

# Force use of system GCC
python3 build.py setup --toolchain=gcc

# Verbose build with 4 parallel jobs
python3 build.py build --verbose --jobs=4
```

### Using Meson Directly

If you prefer to use Meson directly:

```bash
# Configure build
meson setup build --cross-file=i686-elf.cross -Dconfig=debug

# Build
meson compile -C build

# Run specific targets
meson compile -C build run        # Run in QEMU
meson compile -C build debug      # Debug with GDB

# Clean
rm -rf build
```

## Build Targets

The build system creates several targets:

### Libraries
- **libcore** - Core library shared between kernel and bootloader

### Bootloader
- **stage1.bin** - First stage bootloader (fits in boot sector)
- **stage2.bin** - Second stage bootloader (loads kernel)

### Kernel
- **kernel.elf** - Main kernel with debug symbols
- **kernel-stripped.elf** - Stripped kernel (smaller size)

### Final Output
- **NeoOS.img** - Complete bootable disk image

## Configuration Options

Edit `meson_options.txt` or use command-line options:

- `config` - Build type (debug/release)
- `imageType` - Disk image type (hdd/floppy) 
- `imageFS` - Filesystem type (fat12/fat16/fat32)
- `imageSize` - Image size (e.g., "32M", "1G")
- `extra_warnings` - Enable additional compiler warnings
- `static_analysis` - Enable static analysis (requires tools)

## Cross-Compilation

The build system uses cross-compilation files to target the i686 architecture:

- **i686-elf.cross** - For dedicated i686-elf toolchain
- **build-cross.ini** - Auto-generated based on available tools

## Troubleshooting

### Common Issues

**"asm_args unknown keyword"**
- Your Meson version is too old
- Solution: Update to Meson ≥0.56.0 or use the fixed build files

**"i686-elf-gcc not found"**  
- Cross-compiler not installed
- Solution: Install i686-elf-gcc or use `--toolchain=gcc`

**"NASM not found"**
- NASM assembler not installed
- Solution: Install NASM from your package manager

**"Python not found"**
- Python 3 not available or not in PATH
- Solution: Install Python 3 and add to PATH

**Build fails with linker errors**
- Wrong toolchain or missing libraries
- Solution: Ensure you're using the correct cross-compiler

### Debug Tips

1. **Enable verbose output**: `--verbose` flag
2. **Check dependencies**: Run `python3 build.py setup` to verify tools
3. **Clean rebuild**: `python3 build.py rebuild` 
4. **Check logs**: Look at build output for specific error messages
5. **Validate cross-file**: Ensure toolchain binaries are in PATH

### Getting Help

1. Check this documentation
2. Look at error messages carefully  
3. Verify all dependencies are installed
4. Try a clean rebuild
5. Check that your cross-compiler works independently

## Development Workflow

Typical development cycle:

```bash
# Initial setup (once)
python3 build.py setup --config=debug

# Edit code...

# Quick build and test
python3 build.py run

# Debug when needed
python3 build.py debug

# Release build before commits
python3 build.py rebuild --config=release
```

## Performance Tips

- Use `--jobs=N` to enable parallel building
- Release builds are much faster and smaller
- Clean builds when switching configurations
- Use `ninja` backend (default) for best performance

## Integration with IDEs

### Visual Studio Code
1. Install C/C++ extension
2. Configure `.vscode/c_cpp_properties.json` with cross-compiler paths
3. Use integrated terminal for build commands

### CLion
1. Configure as Meson project
2. Set cross-compilation toolchain in settings
3. Use built-in terminal for build commands

The improved build system is more robust, provides better error messages, and should work reliably across different platforms!