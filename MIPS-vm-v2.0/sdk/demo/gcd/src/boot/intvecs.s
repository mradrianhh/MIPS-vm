# Interrupt vector table. Must be placed starting at 0xBFC0.0000
.section .intvecs
.align 4
    .word __StackTop
    .word _reset
