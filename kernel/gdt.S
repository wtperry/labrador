.global gdt_load
gdt_load:
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