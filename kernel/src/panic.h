#ifndef KALUXOS_PANIC_H
#define KALUXOS_PANIC_H

#include <stdint.h>

__attribute__((noreturn))
void panic(const char *message);

__attribute__((noreturn))
void panic_exception(uint64_t vector, uint64_t error);

#endif
