.altmacro   # allows us to use expressions as strings with '%expr'

.macro ISR_ERR_CODE n
_isr_\n\():
    push $\n     #push int number
    jmp isr_common
.endm

.macro ISR_NO_ERR_CODE n
_isr_\n\():
    push $0      #push dummy error code
    push $\n     #push int number
    jmp isr_common
.endm

.macro IRQ n
_isr_\n\():
    push $0      #push dummy error code
    push $\n     #push int number
    jmp isr_common
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
.set i, 32
.rept 224
    IRQ %i
    .set i, i+1
.endr

isr_common:
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
    mov %rsp, %rdi  # exception data struct
    call default_int_handler
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
.macro ISR_LABEL n
    .quad _isr_\n
.endm

.global isr_stub_table
isr_stub_table:
.set i, 0
.rept 256
    ISR_LABEL %i
    .set i, i+1
.endr