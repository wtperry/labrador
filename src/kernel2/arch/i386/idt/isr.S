.altmacro   #allows us to use expressions as strings with '%expr'

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
    pusha           #pushes edi, esi, ebp, esp, ebx, edx, ecx, eax
    push %ds
    push %es
    push %fs
    push %gs
    mov $0x10, %ax   #load kernel data segment descriptor
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %esp, %eax
    push %eax
    call isr_exception_handler
    pop %eax
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popa
    add $8, %esp
    iret

.global isr_stub_table
isr_stub_table:
.long _isr_exception_0
.long _isr_exception_1
.long _isr_exception_2
.long _isr_exception_3
.long _isr_exception_4
.long _isr_exception_5
.long _isr_exception_6
.long _isr_exception_7
.long _isr_exception_8
.long _isr_exception_9
.long _isr_exception_10
.long _isr_exception_11
.long _isr_exception_12
.long _isr_exception_13
.long _isr_exception_14
.long _isr_exception_15
.long _isr_exception_16
.long _isr_exception_17
.long _isr_exception_18
.long _isr_exception_19
.long _isr_exception_20
.long _isr_exception_21
.long _isr_exception_22
.long _isr_exception_23
.long _isr_exception_24
.long _isr_exception_25
.long _isr_exception_26
.long _isr_exception_27
.long _isr_exception_28
.long _isr_exception_29
.long _isr_exception_30
.long _isr_exception_31