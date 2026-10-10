.section .text
.global cpu_halt
.type cpu_halt, @function

cpu_halt:
    cli
1:
    hlt
    jmp 1b

.size cpu_halt, . - cpu_halt

.section .note.GNU-stack,"",@progbits
