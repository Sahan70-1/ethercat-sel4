# Group 19 - seL4 Microkit Boot Experiment
# Microkit SDK: 2.4.0
# Target: qemu_virt_aarch64 (debug)

ifndef MICROKIT_SDK
$(error Set MICROKIT_SDK to the Microkit SDK 2.4.0 directory)
endif

BOARD := qemu_virt_aarch64
CONFIG := debug
BUILD_DIR := build

CC := aarch64-linux-gnu-gcc
LD := aarch64-linux-gnu-ld

BOARD_DIR := $(MICROKIT_SDK)/board/$(BOARD)/$(CONFIG)
MICROKIT_TOOL := $(MICROKIT_SDK)/bin/microkit

# Workaround for ARM64 GCC assembler lookup on Ubuntu
AS_PREFIX ?= /usr/bin/aarch64-linux-gnu-

CFLAGS := -mcpu=cortex-a53 -mstrict-align \
           -nostdlib -fno-stack-protector \
           -ffreestanding -g -Wall \
           -Wno-array-bounds -Wno-unused-variable \
           -Wno-unused-function -Werror \
           -I$(BOARD_DIR)/include

.PHONY: all run clean

all: $(BUILD_DIR)/loader.img

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/serial_server.o: serial_server.c | $(BUILD_DIR)
	$(CC) -B$(AS_PREFIX) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/serial_server.elf: $(BUILD_DIR)/serial_server.o
	$(LD) -L$(BOARD_DIR)/lib $< \
	    -lmicrokit -Tmicrokit.ld -o $@

$(BUILD_DIR)/loader.img: system.system $(BUILD_DIR)/serial_server.elf
	$(MICROKIT_TOOL) system.system \
	    --search-path $(BUILD_DIR) \
	    --board $(BOARD) \
	    --config $(CONFIG) \
	    -o $@ \
	    -r $(BUILD_DIR)/report.txt

run: all
	qemu-system-aarch64 \
	    -machine virt,virtualization=on \
	    -cpu cortex-a53 \
	    -serial mon:stdio \
	    -device loader,file=$(BUILD_DIR)/loader.img,addr=0x70000000,cpu-num=0 \
	    -m size=2G \
	    -nographic

clean:
	rm -rf $(BUILD_DIR)
