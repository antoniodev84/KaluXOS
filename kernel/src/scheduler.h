#ifndef KALUXOS_SCHEDULER_H
#define KALUXOS_SCHEDULER_H
#include <stdint.h>
void scheduler_init(void);
uint64_t *scheduler_tick(uint64_t *context, uint64_t irq);
#endif
