#ifndef KALUXOS_IRQ_H
#define KALUXOS_IRQ_H

#include <stdint.h>

void irq_init(void);
void irq_handler(uint64_t irq);

#endif
