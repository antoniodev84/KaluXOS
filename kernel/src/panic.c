#include "panic.h"
#include "uart.h"

static const char *exception_names[] = {
    "Divide Error",
    "Debug",
    "Non-Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 Floating Point",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating Point",
    "Virtualization",
    "Control Protection",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Hypervisor Injection",
    "VMM Communication",
    "Security Exception",
    "Reserved"
};

__attribute__((noreturn))
void panic(const char *message) {
    __asm__ volatile ("cli");

    uart_puts("\n\n");
    uart_puts("================================\n");
    uart_puts("             KALUXOS PANIC\n");
    uart_puts("================================\n");
    uart_puts(message);
    uart_puts("\n================================\n");

    for (;;) {
        __asm__ volatile ("hlt");
    }
}

__attribute__((noreturn))
void panic_exception(uint64_t vector, uint64_t error) {
    __asm__ volatile ("cli");

    uart_puts("\n\n");
    uart_puts("================================\n");
    uart_puts("          KALUXOS EXCEPTION\n");
    uart_puts("================================\n");
    uart_puts("Vector: ");
    uart_put_dec(vector);
    uart_puts("\nError:  ");
    uart_put_hex(error);
    uart_puts("\n");

    if (vector < 32) {
        uart_puts("Exception: ");
        uart_puts(exception_names[vector]);
        uart_puts("\n");
    }

    uart_puts("================================\n");

    for (;;) {
        __asm__ volatile ("hlt");
    }
}
