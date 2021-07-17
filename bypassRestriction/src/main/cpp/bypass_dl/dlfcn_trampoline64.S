.text
.align 4

.globl dlfcn_trampoline
.type  dlfcn_trampoline, %function

dlfcn_trampoline:
        stp x29, x30, [sp, #-16]!
        mov x29, sp
        mov x30, x2
        br x3