#include "irq_idt.h"
#include "uart.h"

#include <stdint.h>

struct idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t type_attributes;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t zero;
} __attribute__((packed));

extern struct idt_entry idt[256];

extern void irq0(void);
extern void irq1(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);

static void set_gate(uint8_t vector, uint64_t handler) {
    idt[vector].offset_low = handler & 0xffff;
    idt[vector].selector = 0x08;
    idt[vector].ist = 0;
    idt[vector].type_attributes = 0x8e;
    idt[vector].offset_mid = (handler >> 16) & 0xffff;
    idt[vector].offset_high = (handler >> 32) & 0xffffffff;
    idt[vector].zero = 0;
}

void irq_idt_install(void) {
    set_gate(32, (uint64_t)irq0);
    set_gate(33, (uint64_t)irq1);
    set_gate(34, (uint64_t)irq2);
    set_gate(35, (uint64_t)irq3);
    set_gate(36, (uint64_t)irq4);
    set_gate(37, (uint64_t)irq5);
    set_gate(38, (uint64_t)irq6);
    set_gate(39, (uint64_t)irq7);
    set_gate(40, (uint64_t)irq8);
    set_gate(41, (uint64_t)irq9);
    set_gate(42, (uint64_t)irq10);
    set_gate(43, (uint64_t)irq11);
    set_gate(44, (uint64_t)irq12);
    set_gate(45, (uint64_t)irq13);
    set_gate(46, (uint64_t)irq14);
    set_gate(47, (uint64_t)irq15);

    uart_puts("[ OK ] PIC IRQ vectors installed\n");
}
