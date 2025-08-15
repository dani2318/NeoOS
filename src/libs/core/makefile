TARGET_ASMFLAGS += -f elf
TARGET_CFLAGS += -ffreestanding -nostdlib -I.
TARGET_LIBS += -lgcc
TARGET_LINKFLAGS += -nostdlib

HEADERS_C = $(shell find . -type f -name "*.h")

SOURCES_C=$(shell find . -type f -name "*.cpp")
OBJECTS_C=$(patsubst %.cpp, $(BUILD_DIR)/libcore/cpp/%.obj, $(SOURCES_C))

SOURCES_ASM=$(shell find . -type f -name "*.asm")
			
HEADERS_ASM=$(shell find . -type f -name "*.inc")
OBJECTS_ASM=$(patsubst %.asm, $(BUILD_DIR)/libcore/asm/%.obj, $(SOURCES_ASM))

.PHONY: all kernel clean always

all: kernel

libcore:$(BUILD_DIR)/libcore.o 

$(BUILD_DIR)/libcore.o: $(OBJECTS_ASM) $(OBJECTS_C)
	@$(TARGET_LD) $(TARGET_LINKFLAGS) -Wl,-Map=$(BUILD_DIR)/libcore.map -o $@ $^ $(TARGET_LIBS)
	@echo "--> Created  libcore.o"

$(BUILD_DIR)/libcore/cpp/%.obj: %.cpp $(HEADERS_C)
	@mkdir -p $(@D)
	@$(TARGET_CXX) $(TARGET_CFLAGS) -c -o $@ $<
	@echo "--> Compiled: " $<

$(BUILD_DIR)/libcore/asm/%.obj: %.asm $(HEADERS_ASM)
	@mkdir -p $(@D)
	@$(TARGET_ASM) $(TARGET_ASMFLAGS) -o $@ $<
	@echo "--> Compiled: " $<

clean:
	@rm -f $(BUILD_DIR)/kernel.bin