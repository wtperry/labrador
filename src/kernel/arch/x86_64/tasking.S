.extern current_thread

.global switch_task
.type switch_task, @function
switch_task:
    push %rbp
    push %rbx
    push %r12
    push %r13
    push %r14
    push %r15


    # Save current thread stack
    mov %gs:0x0, %rcx
    mov %rsp, (%rcx)

    # Load next thread into current_thread
    mov %rdi, %gs:0x0

    # Load next thread stack
    mov (%rdi), %rsp

    # Load next thread CR3
    mov 8(%rdi), %rbx
    mov %cr3, %rax
    cmp %rax, %rbx
    je 1f
    mov %rbx, %cr3

1:
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %rbx
    pop %rbp

    ret

.global task_start
.type task_start, @function
task_start:
    pop %rdi
    mov %rsp, %rax
    mov %ss, %rcx
    push %rcx
    push %rax
    pushfq
    orq $(1 << 9), (%rsp)
    mov %cs, %rax
    push %rax
    push $call_task
    iretq

.global user_task_start
.type user_task_start, @function
user_task_start:
    pop %rcx
    pushfq
    pop %r11
    orq $(1 << 9), %r11
    sysretq

call_task:
    call *%rdi
