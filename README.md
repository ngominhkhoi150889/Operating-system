# Banh-mi-OS

Banh-mi-OS is an educational x86_64 hobby operating-system kernel written in C and Assembly. It boots through **GRUB 2 using the Multiboot2 protocol**.

## OS identity and release metadata

The root file `OS-release` is the single source of truth for the OS identity and release information:

```ini
NAME="Banh-mi-OS"
PRETTY_NAME="Banh-mi-OS 0.1.0"
ID=banh-mi-os
VERSION="0.1.0"
VERSION_ID="0.1.0"
PLATFORM="x86_64"
ARCHITECTURE="x86_64"
BOOTLOADER="GRUB 2"
KERNEL_PROTOCOL="Multiboot2"
```

GRUB loads this file as a Multiboot2 module named `os-release`. The kernel reads and parses the module at boot, then checks the OS name and architecture before continuing. When the kernel later gains a filesystem, this metadata can be exposed at a conventional path such as `/etc/os-release`.

## Current milestones

- GRUB 2 / Multiboot2 boot path
- 32-bit bootstrap that enables x86_64 long mode
- Identity mapping for the first 4 GiB of physical memory
- Reads OS name, version, platform, and architecture from `OS-release`
- Framebuffer boot banner: `BANH-MI-OS`
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
git clone https://github.com/ngominhkhoi150889/Operating-system.git Banh-mi-OS
cd Banh-mi-OS
make iso
```

The ISO is created at `build/Banh-mi-OS.iso`. The build checks the kernel with `grub-file --is-x86-multiboot2`.

## Run in QEMU

```bash
make run
```

The default run target boots the GRUB ISO using QEMU's BIOS firmware. UEFI boot packaging and testing are a separate next milestone. If QEMU's graphical window is unavailable in WSL, build the ISO with `make iso` and run it in a Linux desktop or configure WSLg.

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
├── OS-release
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

1. Verify the GRUB Multiboot2 boot path and metadata module on QEMU.
2. Add UEFI boot packaging and test with QEMU's UEFI firmware.
3. Read the Multiboot2 memory map and add a physical page allocator.
4. Add interrupt handling, a timer, keyboard input, and a basic shell.
5. Start building a graphical desktop.

## Notes

- This is an educational hobby OS, not a replacement for a production operating system.
- Banh-mi-OS does not yet include multitasking, a filesystem, networking, or hardware drivers.
