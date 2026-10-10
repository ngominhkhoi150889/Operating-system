# KhoiOS

KhoiOS is an educational x86_64 hobby operating-system kernel written in C and Assembly. It now boots through **GRUB 2 using the Multiboot2 protocol**.

## Current milestones

- GRUB 2 / Multiboot2 boot path
- 32-bit bootstrap that enables x86_64 long mode
- Identity mapping for the first 4 GiB of physical memory
- Framebuffer boot banner: `KHOIOS BOOT OK`
- Initial `libk` helpers: `k_memset`, `k_memcpy`, `k_memmove`, `k_strlen`, `k_strcmp`, and `k_strncmp`
- Host-side unit tests for the `libk` helpers

## Build on Ubuntu / WSL 2

Install the build and GRUB tools:

```bash
sudo apt update
sudo apt install -y build-essential grub-common grub-pc-bin grub-efi-amd64-bin xorriso mtools qemu-system-x86
```

Clone and build:

```bash
git clone https://github.com/ngominhkhoi150889/Operating-system.git KhoiOS
cd KhoiOS
make iso
```

The ISO is created at `build/KhoiOS.iso`. The build also checks the kernel with `grub-file --is-x86-multiboot2`.

## Run in QEMU

```bash
make run
```

The default run target boots the GRUB ISO using QEMU's BIOS firmware. The ISO can also include GRUB's UEFI boot support when the UEFI GRUB package is installed. If QEMU's graphical window is unavailable in WSL, build the ISO with `make iso` and run it in a Linux desktop or configure WSLg.

## Run the libk unit tests

These tests run as a normal Linux program and do not boot the OS:

```bash
make test
```

Expected output:

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
├── grub
│   └── grub.cfg
├── kernel
│   ├── linker.ld
│   └── src
│       ├── boot.S
│       ├── cpu.S
│       ├── libk.c
│       ├── libk.h
│       └── main.c
└── tests
    └── test_libk.c
```

## Roadmap

1. Verify the GRUB Multiboot2 boot path on QEMU.
2. Read the Multiboot2 memory map and add a physical page allocator.
3. Add interrupt handling and a timer.
4. Add keyboard input and a basic shell.
5. Start building a graphical desktop.

## Notes

- This is an educational hobby OS, not a replacement for a production operating system.
- KhoiOS does not yet include multitasking, a filesystem, networking, or hardware drivers.
