SHELL := /bin/bash

BUILD_DIR := build
ISO_ROOT := $(BUILD_DIR)/iso
KERNEL := $(BUILD_DIR)/kernel.elf
HOST_CC ?= cc
HOST_CFLAGS ?= -std=c11 -Wall -Wextra -Werror -O2
KERNEL_CC ?= gcc
KERNEL_CFLAGS := -std=gnu11 -O2 -Wall -Wextra -Werror \
	-ffreestanding -fno-builtin -fno-stack-protector -fno-pic -fno-pie \
	-fno-asynchronous-unwind-tables -fno-unwind-tables \
	-m64 -mno-red-zone -mcmodel=small

.PHONY: all iso run test clean check-tools

all: iso

check-tools:
	@for tool in $(KERNEL_CC) ld grub-file grub-mkrescue xorriso; do \
		command -v $$tool >/dev/null 2>&1 || { echo "Missing dependency: $$tool"; exit 1; }; \
	done

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/boot.o: kernel/src/boot.S | $(BUILD_DIR)
	$(KERNEL_CC) -m64 -c $< -o $@

$(BUILD_DIR)/cpu.o: kernel/src/cpu.S | $(BUILD_DIR)
	$(KERNEL_CC) -m64 -c $< -o $@

$(BUILD_DIR)/main.o: kernel/src/main.c kernel/src/libk.h | $(BUILD_DIR)
	$(KERNEL_CC) $(KERNEL_CFLAGS) -c $< -o $@

$(BUILD_DIR)/libk.o: kernel/src/libk.c kernel/src/libk.h | $(BUILD_DIR)
	$(KERNEL_CC) $(KERNEL_CFLAGS) -c $< -o $@

$(KERNEL): $(BUILD_DIR)/boot.o $(BUILD_DIR)/cpu.o $(BUILD_DIR)/main.o $(BUILD_DIR)/libk.o kernel/linker.ld
	ld -m elf_x86_64 -T kernel/linker.ld -nostdlib -z max-page-size=0x1000 \
		-o $@ $(BUILD_DIR)/boot.o $(BUILD_DIR)/cpu.o $(BUILD_DIR)/main.o $(BUILD_DIR)/libk.o
	grub-file --is-x86-multiboot2 $@

iso: check-tools $(KERNEL) grub/grub.cfg OS-release
	mkdir -p $(ISO_ROOT)/boot/grub
	cp $(KERNEL) $(ISO_ROOT)/boot/kernel.elf
	cp OS-release $(ISO_ROOT)/boot/OS-release
	cp grub/grub.cfg $(ISO_ROOT)/boot/grub/grub.cfg
	grub-mkrescue -o $(BUILD_DIR)/Banh-mi-OS.iso $(ISO_ROOT)
	@echo
	@echo "GRUB ISO created: $(BUILD_DIR)/Banh-mi-OS.iso"

run: iso
	qemu-system-x86_64 -m 512M -cdrom $(BUILD_DIR)/Banh-mi-OS.iso -no-reboot -no-shutdown

test: | $(BUILD_DIR)
	@mkdir -p $(BUILD_DIR)/tests
	$(HOST_CC) $(HOST_CFLAGS) -Ikernel/src kernel/src/libk.c tests/test_libk.c -o $(BUILD_DIR)/tests/test_libk
	./$(BUILD_DIR)/tests/test_libk

clean:
	rm -rf $(BUILD_DIR)
