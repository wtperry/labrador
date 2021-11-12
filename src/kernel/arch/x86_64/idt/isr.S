.altmacro   # allows us to use expressions as strings with '%expr'

.macro ISR_ERR_CODE n
_isr_exception_\n\():
    push $\n     #push int number
    jmp isr_exception_common
.endm

.macro ISR_NO_ERR_CODE n
_isr_exception_\n\():
    push $0      #push dummy error code
    push $\n     #push int number
    jmp isr_exception_common
.endm

.section .text
ISR_NO_ERR_CODE 0
ISR_NO_ERR_CODE 1
ISR_NO_ERR_CODE 2
ISR_NO_ERR_CODE 3
ISR_NO_ERR_CODE 4
ISR_NO_ERR_CODE 5
ISR_NO_ERR_CODE 6
ISR_NO_ERR_CODE 7
ISR_ERR_CODE 8
ISR_NO_ERR_CODE 9
ISR_ERR_CODE 10
ISR_ERR_CODE 11
ISR_ERR_CODE 12
ISR_ERR_CODE 13
ISR_ERR_CODE 14
ISR_NO_ERR_CODE 15
ISR_NO_ERR_CODE 16
ISR_NO_ERR_CODE 17
ISR_NO_ERR_CODE 18
ISR_NO_ERR_CODE 19
ISR_NO_ERR_CODE 20
ISR_NO_ERR_CODE 21
ISR_NO_ERR_CODE 22
ISR_NO_ERR_CODE 23
ISR_NO_ERR_CODE 24
ISR_NO_ERR_CODE 25
ISR_NO_ERR_CODE 26
ISR_NO_ERR_CODE 27
ISR_NO_ERR_CODE 28
ISR_NO_ERR_CODE 29
ISR_NO_ERR_CODE 30
ISR_NO_ERR_CODE 31

isr_exception_common:
    push %rdi
    push %rsi
    push %rbp
    push %rsp
    push %rbx
    push %rdx
    push %rcx
    push %rax
    push %r8
    push %r9
    push %r10
    push %r11
    push %r12
    push %r13
    push %r14
    push %r15
    mov %ds, %rax
    push %rax
    mov %es, %rax
    push %rax
    mov %fs, %rax
    push %rax
    mov %gs, %rax
    push %rax
    mov %cr2, %rax
    push %rax
    mov $0x10, %ax  # load kernel data segment descriptor
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %rsp, %rdi  # exception data struct
    call isr_exception_handler
    pop %rax
    pop %rax
    mov %ax, %gs
    pop %rax
    mov %ax, %fs
    pop %rax
    mov %ax, %es
    pop %rax
    mov %ax, %ds
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %r11
    pop %r10
    pop %r9
    pop %r8
    pop %rax
    pop %rcx
    pop %rdx
    pop %rbx
    pop %rsp
    pop %rbp
    pop %rsi
    pop %rdi
    add $16, %rsp
    iretq

.section .data
.global isr_stub_table
isr_stub_table:
.quad _isr_exception_0
.quad _isr_exception_1
.quad _isr_exception_2
.quad _isr_exception_3
.quad _isr_exception_4
.quad _isr_exception_5
.quad _isr_exception_6
.quad _isr_exception_7
.quad _isr_exception_8
.quad _isr_exception_9
.quad _isr_exception_10
.quad _isr_exception_11
.quad _isr_exception_12
.quad _isr_exception_13
.quad _isr_exception_14
.quad _isr_exception_15
.quad _isr_exception_16
.quad _isr_exception_17
.quad _isr_exception_18
.quad _isr_exception_19
.quad _isr_exception_20
.quad _isr_exception_21
.quad _isr_exception_22
.quad _isr_exception_23
.quad _isr_exception_24
.quad _isr_exception_25
.quad _isr_exception_26
.quad _isr_exception_27
.quad _isr_exception_28
.quad _isr_exception_29
.quad _isr_exception_30
.quad _isr_exception_31