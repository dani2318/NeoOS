TARGET_ASMFLAGS += -f elf
TARGET_CFLAGS += -ffreestanding -nostdlib -I$(shell pwd)/toolchain/i686-elf/lib/gcc/i686-elf/15.1.0/include
TARGET_LIBS += -lgcc
TARGET_LINKFLAGS += -T linker.ld -nostdlib

SOURCES_C= $(wildcard *.c)
OBJECTS_C := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/stage2/c/%.obj, $(SOURCES_C))

SOURCES_ASM=$(wildcard *.asm)
OBJECTS_ASM=$(patsubst %.asm, $(BUILD_DIR)/stage2/asm/%.obj, $(SOURCES_ASM))

.PHONY: all stage2 clean always

all: stage2

list_c: $(SOURCES_C)
	@echo "-->" $<

stage2: $(BUILD_DIR)/stage2.bin
	@echo $(SOURCES_C)

$(BUILD_DIR)/stage2.bin: $(OBJECTS_ASM) $(OBJECTS_C)
	$(TARGET_LD) $(TARGET_LINKFLAGS) -Wl,-Map=$(BUILD_DIR)/stage2.map -o $@ $^ $(TARGET_LIBS)
	@echo "--> Created  stage2.bin"

$(BUILD_DIR)/stage2/c/%.obj: %.c 
	@mkdir -p $(@D) && $(TARGET_CC) $(TARGET_CFLAGS) -c -o $@ $<
	@echo "--> Compiled: " $<

$(BUILD_DIR)/stage2/asm/%.obj: %.asm
	@mkdir -p $(@D) 
	$(TARGET_ASM) $(TARGET_ASMFLAGS) -o $@ $<
	@echo "--> Compiled: " $<

clean:
	@rm -f $(BUILD_DIR)/stage2.bin