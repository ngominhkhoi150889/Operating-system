SHELL := /bin/bash

TEMPLATE_REPO := https://github.com/Limine-Bootloader/limine-c-template-x86-64.git
BUILD_DIR := build/template
KERNEL_DIR := $(BUILD_DIR)/kernel

.PHONY: all iso run clean prepare

all: iso

prepare:
	@command -v git >/dev/null || { echo "Missing dependency: git"; exit 1; }
	@if [ ! -d "$(BUILD_DIR)/.git" ]; then \
		mkdir -p build; \
		git clone --depth 1 "$(TEMPLATE_REPO)" "$(BUILD_DIR)"; \
	fi
	@cp kernel/src/main.c "$(KERNEL_DIR)/src/main.c"
	@cp kernel/src/cpu.S "$(KERNEL_DIR)/src/cpu.S"
	@sed -i 's/^override IMAGE_NAME := template/override IMAGE_NAME := KhoiOS/' "$(BUILD_DIR)/GNUmakefile"
	@sed -i 's/Limine Template/KhoiOS/' "$(BUILD_DIR)/limine.conf"

iso: prepare
	$(MAKE) -C "$(BUILD_DIR)" all
	@echo
	@echo "ISO created: $(BUILD_DIR)/KhoiOS.iso"

run: iso
	$(MAKE) -C "$(BUILD_DIR)" run-uefi

clean:
	@if [ -d "$(BUILD_DIR)" ]; then $(MAKE) -C "$(BUILD_DIR)" clean; fi
	rm -rf build
