# KhoiOS

KhoiOS is a small x86_64 hobby operating-system kernel written in C and Assembly. It boots through the Limine boot protocol in UEFI mode (QEMU + OVMF).

## Current milestone

- Limine-based boot
- Freestanding C kernel
- x86_64 Assembly CPU halt routine
- Limine terminal output: "Welcome to KhoiOS!"
- QEMU/OVMF test target

This repository keeps KhoiOS source code separate from the upstream Limine C template. The Makefile fetches the upstream template into `build/template` and injects the KhoiOS kernel sources before building. The template's own build scripts fetch the Limine protocol/dependency files and create the ISO.

## Build on Ubuntu / WSL 2

Install the host tools:

```bash
sudo apt update
sudo apt install -y build-essential git curl xorriso qemu-system-x86 ovmf mtools nasm
```

Clone and build:

```bash
git clone https://github.com/ngominhkhoi150889/Operating-system.git KhoiOS
cd KhoiOS
make iso
```

The resulting image is at `build/template/KhoiOS.iso`.

## Run with UEFI

```bash
make run
```

This uses QEMU with OVMF (UEFI firmware). Close the QEMU window to exit.

If you are using WSL without GUI support, run `make iso` first and then run QEMU from a desktop Linux environment, or configure WSLg.

## Clean generated files

```bash
make clean
```

## Project layout

```text
.
├── Makefile
├── README.md
└── kernel
    └── src
        ├── main.c
        └── cpu.S
```

## Roadmap

1. Confirm UEFI boot and terminal output.
2. Read the Limine memory map and add physical page allocation.
3. Add interrupt handling and a timer.
4. Add keyboard input and a basic shell.

## Notes

- This is an educational hobby OS, not a replacement for a production operating system.
- The first milestone does not yet include multitasking, a filesystem, networking, or hardware drivers.
- Limine C template: https://github.com/Limine-Bootloader/limine-c-template-x86-64
