# KhoiOS

KhoiOS is an educational x86_64 hobby operating-system kernel written in C and Assembly. It boots through the Limine boot protocol in UEFI mode (QEMU + OVMF).

## Current milestones

- Limine UEFI boot path
- Freestanding C kernel with a framebuffer boot banner
- x86_64 Assembly CPU halt routine
- Initial `libk` helpers: `k_memset`, `k_memcpy`, `k_memmove`, `k_strlen`, `k_strcmp`, and `k_strncmp`
- Host-side unit tests for the `libk` helpers

This repository keeps KhoiOS source code separate from the upstream Limine C template. The Makefile fetches the upstream template into `build/template` and copies in KhoiOS kernel sources before building. The template's own build scripts fetch the Limine protocol/dependency files and create the ISO.

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

This uses QEMU with OVMF (UEFI firmware). Close the QEMU window to exit. If you are using WSL without GUI support, run `make iso` first and then run QEMU from a desktop Linux environment, or configure WSLg.

## Run the libk unit tests

These tests run as a normal Linux program and do not boot the OS:

```bash
make test
```

A successful run prints:

```text
KhoiOS libk tests: PASS
```

## Clean generated files

```bash
make clean
```

## Project layout

```text
.
├── Makefile
├── README.md
├── kernel
│   └── src
│       ├── main.c
│       ├── cpu.S
│       ├── libk.c
│       └── libk.h
└── tests
    └── test_libk.c
```

## Roadmap

1. Build and test the initial kernel support library.
2. Read the Limine memory map and add physical page allocation.
3. Add interrupt handling and a timer.
4. Add keyboard input and a basic shell.
5. Start building a graphical desktop.

## Notes

- This is an educational hobby OS, not a replacement for a production operating system.
- KhoiOS does not yet include multitasking, a filesystem, networking, or hardware drivers.
- Limine C template: https://github.com/Limine-Bootloader/limine-c-template-x86-64
