#include "scheduler.h"
#include "pmm.h"
#include "vmm.h"
#include "heap.h"
#include "memory.h"
#include "pic.h"
#include "pit.h"
#include "uart.h"
#include <stdint.h>

#define USER_CODE 0x0000004000000000ULL
#define USER_MESSAGE 0x0000004000001000ULL
#define USER_TIMESPEC 0x0000004000002000ULL
#define USER_STACK 0x700000000000ULL
#define USER_CS 0x23ULL
#define USER_DS 0x1bULL
#define TASK_COUNT 2
#define SCHEDULER_QUANTUM 10

typedef struct task {
    uint64_t *context;
    uint64_t ticks;
} task_t;

static task_t tasks[TASK_COUNT];
static uint32_t current_task;
static uint8_t user_code[] = {
    0xb8, 0x01, 0x00, 0x00, 0x00,
    0xbf, 0x01, 0x00, 0x00, 0x00,
    0x48, 0xbe, 0x00, 0x10, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,
    0xba, 0x1d, 0x00, 0x00, 0x00,
    0x0f, 0x05,
    0xb8, 0x23, 0x00, 0x00, 0x00,
    0x48, 0xbf, 0x00, 0x20, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,
    0x0f, 0x05,
    0xeb, 0xfe
};
static const char user_message[] = "[ring3] write via syscall\n";

static uint64_t *make_user_context(void) {
    uint64_t code_physical = pmm_alloc_page();
    uint64_t message_physical = pmm_alloc_page();
    uint64_t timespec_physical = pmm_alloc_page();
    uint64_t stack_physical = pmm_alloc_page();
    uint64_t *kernel_stack = kmalloc(4096);
    if (code_physical == 0 || message_physical == 0 || timespec_physical == 0 || stack_physical == 0 || kernel_stack == 0) {
        return 0;
    }
    if (vmm_map(USER_CODE, code_physical, VMM_USER) != 0 ||
        vmm_map(USER_MESSAGE, message_physical, VMM_WRITABLE | VMM_USER | VMM_NO_EXECUTE) != 0 ||
        vmm_map(USER_TIMESPEC, timespec_physical, VMM_WRITABLE | VMM_USER | VMM_NO_EXECUTE) != 0 ||
        vmm_map(USER_STACK, stack_physical, VMM_WRITABLE | VMM_USER | VMM_NO_EXECUTE) != 0) {
        return 0;
    }
    memcpy(vmm_phys_to_virt(code_physical), user_code, sizeof(user_code));
    memcpy(vmm_phys_to_virt(message_physical), user_message, sizeof(user_message));
    uint64_t *timespec = vmm_phys_to_virt(timespec_physical);
    timespec[0] = 0;
    timespec[1] = 50000000;
    uint64_t *context = (uint64_t *)((uintptr_t)kernel_stack + 4096 - 21 * sizeof(uint64_t));
    memset(context, 0, 21 * sizeof(uint64_t));
    context[15] = 0;
    context[16] = USER_CODE;
    context[17] = USER_CS;
    context[18] = 0x202;
    context[19] = USER_STACK + VMM_PAGE_SIZE - 16;
    context[20] = USER_DS;
    return context;
}

void scheduler_init(void) {
    current_task = 0;
    tasks[0].context = 0;
    tasks[0].ticks = 0;
    tasks[1].context = make_user_context();
    tasks[1].ticks = 0;
    if (tasks[1].context == 0) {
        uart_puts("[ ERR ] Scheduler user task unavailable\n");
        return;
    }
    uart_puts("[ OK ] Preemptive scheduler initialized, quantum: 10 ticks\n");
    uart_puts("[ OK ] Ring 3 task initialized\n");
}

uint64_t *scheduler_tick(uint64_t *context, uint64_t irq) {
    if (current_task < TASK_COUNT) {
        tasks[current_task].context = context;
        tasks[current_task].ticks++;
    }
    if (irq == 0) {
        pit_tick();
        if (tasks[1].context != 0 && tasks[current_task].ticks >= SCHEDULER_QUANTUM) {
            tasks[current_task].ticks = 0;
            current_task = (current_task + 1) % TASK_COUNT;
        }
    }
    pic_send_eoi((uint8_t)irq);
    if (current_task == 0 || tasks[current_task].context == 0) {
        return context;
    }
    return tasks[current_task].context;
}
