.section .bss
.align 16
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

.section .text
.global _start
.type _start, @function
_start:
    # Setup Stack
    movq $stack_top, %rsp
    xor %rbp, %rbp

    # Save boot info to pass to kernel
    push %rdi

    call init_gdt
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