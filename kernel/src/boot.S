/* GRUB Multiboot2 entry: start in 32-bit protected mode, enable long mode,
 * identity-map the first 4 GiB with 2 MiB pages, then call the C kernel. */

.set MB2_HEADER_MAGIC, 0xe85250d6
.set MB2_BOOT_MAGIC,   0x36d76289
.set CR4_PAE,          0x20
.set EFER_MSR,         0xc0000080
.set EFER_LME,         0x100
.set CR0_PG,           0x80000000
.set PAGE_PRESENT_WRITE_HUGE, 0x83

.section .multiboot,"a"
.align 8
header_start:
    .long MB2_HEADER_MAGIC
    .long 0
    .long header_end - header_start
    .long -(MB2_HEADER_MAGIC + (header_end - header_start))

    /* Request a 1024x768x32 framebuffer when available. */
    .short 5
    .short 0
    .long 20
    .long 1024
    .long 768
    .long 32

    .align 8
    /* End tag. */
    .short 0
    .short 0
    .long 8
header_end:

.section .text.boot,"ax"
.code32
.global _start
.type _start, @function
_start:
    cli
    movl %eax, boot_magic
    movl %ebx, boot_info_phys

    /* Four page directories map physical addresses 0..4 GiB. */
    movl $pdpt_table, %eax
    orl $0x3, %eax
    movl %eax, pml4_table

    movl $pd0, %eax
    orl $0x3, %eax
    movl %eax, pdpt_table + 0
    movl $pd1, %eax
    orl $0x3, %eax
    movl %eax, pdpt_table + 8
    movl $pd2, %eax
    orl $0x3, %eax
    movl %eax, pdpt_table + 16
    movl $pd3, %eax
    orl $0x3, %eax
    movl %eax, pdpt_table + 24

    /* Fill 2048 2 MiB page-directory entries. */
    xorl %ecx, %ecx
1:
    movl %ecx, %eax
    shll $21, %eax
    orl $PAGE_PRESENT_WRITE_HUGE, %eax
    movl %eax, pd0(,%ecx,8)
    movl $0, pd0+4(,%ecx,8)
    incl %ecx
    cmpl $2048, %ecx
    jne 1b

    /* Enable PAE, long mode, paging, and load the 64-bit GDT. */
    movl %cr4, %eax
    orl $CR4_PAE, %eax
    movl %eax, %cr4

    movl $pml4_table, %eax
    movl %eax, %cr3

    movl $EFER_MSR, %ecx
    rdmsr
    orl $EFER_LME, %eax
    wrmsr

    movl %cr0, %eax
    orl $CR0_PG, %eax
    movl %eax, %cr0

    lgdt gdt64_pointer
    ljmp $0x08, $long_mode_start

.code64
long_mode_start:
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %ss
    xorw %ax, %ax
    movw %ax, %fs
    movw %ax, %gs

    leaq stack_top(%rip), %rsp
    andq $-16, %rsp
    movl boot_info_phys(%rip), %edi
    movl boot_magic(%rip), %esi
    call kmain

2:
    cli
    hlt
    jmp 2b

.size _start, . - _start

.section .rodata,"a"
.align 8
gdt64:
    .quad 0x0000000000000000
    .quad 0x00af9a000000ffff
    .quad 0x00cf92000000ffff
gdt64_end:
gdt64_pointer:
    .word gdt64_end - gdt64 - 1
    .long gdt64

.section .bss,"aw",@nobits
.align 4096
pml4_table:
    .skip 4096
pdpt_table:
    .skip 4096
pd0:
    .skip 4096
pd1:
    .skip 4096
pd2:
    .skip 4096
pd3:
    .skip 4096

.align 16
stack_bottom:
    .skip 16384
stack_top:

.align 4
boot_info_phys:
    .skip 4
boot_magic:
    .skip 4

.section .note.GNU-stack,"",@progbits
