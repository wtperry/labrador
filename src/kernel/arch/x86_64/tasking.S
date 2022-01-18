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
    mov (current_thread), %rcx
    mov %rsp, (%rcx)

    # Load next thread into current_thread
    mov %rdi, (current_thread)

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
    pop %rax
    mov $(1 << 9), %rcx
    or %rcx, %rax
    push %rax
    mov %cs, %rax
    push %rax
    push $call_task
    iretq

call_task:
    call %rdi
