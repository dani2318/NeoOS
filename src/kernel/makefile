TARGET_ASMFLAGS += -f elf -Isrc/kernel
TARGET_CFLAGS += -ffreestanding -nostdlib -I. -I $(shell pwd)/toolchain/i686-elf/lib/gcc/i686-elf/15.1.0/include
TARGET_LIBS += -lgcc
TARGET_LINKFLAGS += -T linker.ld -nostdlib

HEADERS_C = $(shell find . -type f -name "*.h")

SOURCES_C=$(shell find . -type f -name "*.c")
OBJECTS_C=$(patsubst %.c, $(BUILD_DIR)/kernel/c/%.obj, $(SOURCES_C))

SOURCES_ASM=$(shell find . -type f -name "*.asm")
			
HEADERS_ASM=$(shell find . -type f -name "*.inc")
OBJECTS_ASM=$(patsubst %.asm, $(BUILD_DIR)/kernel/asm/%.obj, $(SOURCES_ASM))

.PHONY: all kernel clean always generate_isr

all: kernel

kernel:generate_isr $(BUILD_DIR)/kernel.elf 

$(BUILD_DIR)/kernel.elf: $(OBJECTS_ASM) $(OBJECTS_C)
	@$(TARGET_LD) $(TARGET_LINKFLAGS) -Wl,-Map=$(BUILD_DIR)/kernel.map -o $@ $^ $(TARGET_LIBS)
	@echo "--> Created  kernel.elf"

$(BUILD_DIR)/kernel/c/%.obj: %.c $(HEADERS_C)
	@mkdir -p $(@D) 
	@echo $(SOURCES_C)
	$(TARGET_CC) $(TARGET_CFLAGS) -c -o $@ $<
	@echo "--> Compiled: " $<

$(BUILD_DIR)/kernel/asm/%.obj: %.asm $(HEADERS_ASM)
	@mkdir -p $(@D)
	@$(TARGET_ASM) $(TARGET_ASMFLAGS) -o $@ $<
	@echo "--> Compiled: " $<

generate_isr:
	$(shell python3 ../../scripts/generate_isr.py)

clean:
	@rm -f $(BUILD_DIR)/kernel.elf