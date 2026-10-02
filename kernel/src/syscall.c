#include "syscall.h"
#include "pit.h"
#include "uart.h"
#include <stdint.h>

#define SYS_WRITE 1
#define SYS_USLEEP 35
#define EBADF 9
#define EFAULT 14
#define EINVAL 22
#define ENOSYS 38
#define USER_MAX 0x0000800000000000ULL

typedef struct linux_timespec {
    int64_t seconds;
    int64_t nanoseconds;
} linux_timespec_t;

static int64_t sys_write(uint64_t fd, const char *buffer, uint64_t count) {
    if (fd != 1 && fd != 2) {
        return -EBADF;
    }
    if (buffer == 0 || (uint64_t)(uintptr_t)buffer >= USER_MAX || count > 4096) {
        return -EFAULT;
    }
    for (uint64_t i = 0; i < count; i++) {
        uart_putc(buffer[i]);
    }
    return (int64_t)count;
}

static int64_t sys_usleep(const linux_timespec_t *request) {
    if (request == 0 || (uint64_t)(uintptr_t)request >= USER_MAX || request->seconds < 0 || request->nanoseconds < 0 || request->nanoseconds >= 1000000000) {
        return -EINVAL;
    }
    uint64_t start = pit_get_ticks();
    uint64_t nanoseconds = (uint64_t)request->seconds * 1000000000ULL + (uint64_t)request->nanoseconds;
    uint64_t ticks = (nanoseconds + 9999999) / 10000000;
    if (ticks == 0 && nanoseconds != 0) {
        ticks = 1;
    }
    __asm__ volatile ("sti");
    while ((pit_get_ticks() - start) < ticks) {
        __asm__ volatile ("hlt");
    }
    __asm__ volatile ("cli");
    return 0;
}

void syscall_init(void) {
    uart_puts("[ OK ] SYSCALL initialized, write: 1, usleep/nanosleep: 35\n");
}

void syscall_dispatch(syscall_frame_t *frame) {
    switch (frame->rax) {
        case SYS_WRITE:
            frame->rax = (uint64_t)sys_write(frame->rdi, (const char *)(uintptr_t)frame->rsi, frame->rdx);
            break;
        case SYS_USLEEP:
            frame->rax = (uint64_t)sys_usleep((const linux_timespec_t *)(uintptr_t)frame->rdi);
            break;
        default:
            frame->rax = (uint64_t)-ENOSYS;
            break;
    }
}
