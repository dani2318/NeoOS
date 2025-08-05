export CFLAGS = -std=c99 -g
export ASMFLAGS =
export CC = gcc
export CXX = g++
export LD = gcc
export ASM = nasm
export LINKFLAGS =
export LIBS =
export TARGET_TOOLCHAIN_PATH = $(abspath toolchain)
export TARGET = i686-elf
export TARGET_ASM = nasm
export TARGET_ASMFLAGS =
export TARGET_CFLAGS = -std=c99 -g #-O2
export TARGET_CC = /mnt/e/NeoOS/toolchain/i686-elf/bin/i686-elf-gcc
export TARGET_CXX = /mnt/e/NeoOS/toolchain/i686-elf/bin/i686-elf-g++
export TARGET_LD = /mnt/e/NeoOS/toolchain/i686-elf/bin/i686-elf-gcc
export TARGET_LINKFLAGS =
export TARGET_LIBS =

export BUILD_DIR = $(abspath build)
export SRC_DIR = $(abspath src)