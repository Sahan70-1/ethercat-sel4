# Group 19 - Experiment 03: IPC Round-Trip Latency
# Microkit SDK: 2.4.0
# Target: QEMU AArch64

ifndef MICROKIT_SDK
$(error Set MICROKIT_SDK to the Microkit SDK directory)
endif

BOARD := qemu_virt_aarch64
CONFIG := debug
BUILD_DIR := build

CC := aarch64-linux-gnu-gcc
LD := aarch64-linux-gnu-ld

BOARD_DIR := $(MICROKIT_SDK)/board/$(BOARD)/$(CONFIG)
MICROKIT_TOOL := $(MICROKIT_SDK)/bin/microkit

AS_PREFIX ?= /usr/bin/aarch64-linux-gnu-

CFLAGS := -mcpu=cortex-a53 -mstrict-align \
           -nostdlib -fno-stack-protector \
           -ffreestanding -O2 -g -Wall -Werror \
           -I$(BOARD_DIR)/include

.PHONY: all run clean

all: $(BUILD_DIR)/loader.img

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) -B$(AS_PREFIX) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.elf: $(BUILD_DIR)/%.o
	$(LD) -L$(BOARD_DIR)/lib $< \
	    -lmicrokit -Tmicrokit.ld -o $@

$(BUILD_DIR)/loader.img: system.system \
                         $(BUILD_DIR)/master.elf \
                         $(BUILD_DIR)/driver.elf
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
