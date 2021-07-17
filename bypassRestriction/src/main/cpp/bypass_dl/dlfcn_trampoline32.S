.text
.align 4
.code 16

.globl dlfcn_trampoline
.type  dlfcn_trampoline, %function

dlfcn_trampoline:
    push {r4, lr}
    mov lr, r2
    mov pc, r3