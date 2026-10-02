#include "gdt.h"
#include "syscall.h"
#include <stdint.h>

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed));

struct tss_entry {
    uint32_t reserved0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iomap_base;
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

static struct gdt_entry gdt[7];
static struct tss_entry tss;
static struct gdt_ptr gdtr;
static uint8_t kernel_stack[16384] __attribute__((aligned(16)));
uint64_t syscall_kernel_stack;
uint64_t syscall_user_rsp;

static void gdt_set_gate(int index, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity) {
    gdt[index].base_low = base & 0xffff;
    gdt[index].base_middle = (base >> 16) & 0xff;
    gdt[index].base_high = (base >> 24) & 0xff;
    gdt[index].limit_low = limit & 0xffff;
    gdt[index].granularity = ((limit >> 16) & 0x0f) | (granularity & 0xf0);
    gdt[index].access = access;
}

static void gdt_set_tss(void) {
    uint64_t base = (uint64_t)(uintptr_t)&tss;
    uint64_t limit = sizeof(tss) - 1;
    gdt[5].limit_low = limit & 0xffff;
    gdt[5].base_low = base & 0xffff;
    gdt[5].base_middle = (base >> 16) & 0xff;
    gdt[5].access = 0x89;
    gdt[5].granularity = (limit >> 16) & 0x0f;
    gdt[5].base_high = (base >> 24) & 0xff;
    gdt[6].limit_low = (base >> 32) & 0xffff;
    gdt[6].base_low = (base >> 48) & 0xffff;
    gdt[6].base_middle = 0;
    gdt[6].access = 0;
    gdt[6].granularity = 0;
    gdt[6].base_high = 0;
}

static void write_msr(uint32_t msr, uint64_t value) {
    uint32_t low = value & 0xffffffff;
    uint32_t high = value >> 32;
    __asm__ volatile ("wrmsr" : : "c"(msr), "a"(low), "d"(high));
}

static uint64_t read_msr(uint32_t msr) {
    uint32_t low;
    uint32_t high;
    __asm__ volatile ("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
    return ((uint64_t)high << 32) | low;
}

void gdt_init(void) {
    gdtr.limit = sizeof(gdt) - 1;
    gdtr.base = (uint64_t)(uintptr_t)&gdt;
    gdt_set_gate(0, 0, 0, 0, 0);
    gdt_set_gate(1, 0, 0xffffffff, 0x9a, 0x20);
    gdt_set_gate(2, 0, 0xffffffff, 0x92, 0x00);
    gdt_set_gate(3, 0, 0xffffffff, 0xf2, 0x00);
    gdt_set_gate(4, 0, 0xffffffff, 0xfa, 0x20);
    tss = (struct tss_entry){0};
    tss.rsp0 = (uint64_t)(uintptr_t)(kernel_stack + sizeof(kernel_stack));
    tss.iomap_base = sizeof(tss);
    gdt_set_tss();
    syscall_kernel_stack = tss.rsp0;
    __asm__ volatile (
        "lgdt %0\n"
        "pushq $0x08\n"
        "leaq 1f(%%rip), %%rax\n"
        "pushq %%rax\n"
        "lretq\n"
        "1:\n"
        "movw $0x10, %%ax\n"
        "movw %%ax, %%ds\n"
        "movw %%ax, %%es\n"
        "movw %%ax, %%ss\n"
        "movw $0x28, %%ax\n"
        "ltr %%ax\n"
        :
        : "m"(gdtr)
        : "rax", "memory"
    );
    write_msr(0xc0000081, (0x13ULL << 48) | (0x08ULL << 32));
    write_msr(0xc0000082, (uint64_t)(uintptr_t)syscall_entry);
    write_msr(0xc0000084, 0x200);
    write_msr(0xc0000080, read_msr(0xc0000080) | 1);
}
