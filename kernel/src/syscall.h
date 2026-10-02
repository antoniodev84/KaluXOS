#ifndef KALUXOS_SYSCALL_H
#define KALUXOS_SYSCALL_H
#include <stdint.h>

typedef struct syscall_frame {
    uint64_t rax;
    uint64_t rbx;
    uint64_t rbp;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rdx;
    uint64_t r10;
    uint64_t r8;
    uint64_t r9;
    uint64_t rcx;
    uint64_t r11;
    uint64_t rsp;
} syscall_frame_t;

void syscall_init(void);
void syscall_dispatch(syscall_frame_t *frame);
void syscall_entry(void);
#endif
