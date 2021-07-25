.section .bss
.align 16
stack_bottom:
.skip 4096 # 4 KiB
stack_top:

.section .data
gdt64:
.quad 0x00
.quad 0x00A09A0000000000
.quad 0x00A0920000000000
.quad 0x00A0FA0000000000
.quad 0x00A0F20000000000

gdt_ptr:
.word gdt_ptr - gdt64 - 1
.quad gdt64

.section .text
.global _start
.type _start, @function
_start:
    # Setup Stack
    movq $stack_top, %rsp
    xor %rbp, %rbp

    # Save boot info to pass to kernel
    push %rdi

    movq $gdt_ptr, %rdi
    call load_gdt

    call init_idt

    # Call kernel
    pop %rdi
    call kernel_main

    cli
1:  hlt
    jmp 1b

.global load_gdt
load_gdt:
	lgdt (%rdi)

	mov $0x10, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	mov %ax, %ss

    # Setup long return to set cs
	pop %rdi
    mov $0x08, %rax
    push %rax
    push %rdi
	retfq

.global load_idt
load_idt:
	lidt (%rdi)
	ret