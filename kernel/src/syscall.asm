bits 64
section .text
extern syscall_dispatch
extern syscall_kernel_stack
extern syscall_user_rsp
global syscall_entry
syscall_entry:
    cli
    mov [rel syscall_user_rsp], rsp
    mov rsp, [rel syscall_kernel_stack]
    push qword [rel syscall_user_rsp]
    push r11
    push rcx
    push r9
    push r8
    push r10
    push rdx
    push rsi
    push rdi
    push rbp
    push rbx
    push rax
    mov rdi, rsp
    call syscall_dispatch
    pop rax
    pop rbx
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop r10
    pop r8
    pop r9
    pop rcx
    pop r11
    pop rsp
    db 0x48, 0x0f, 0x07
