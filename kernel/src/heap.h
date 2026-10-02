#ifndef KALUXOS_HEAP_H
#define KALUXOS_HEAP_H
#include <stddef.h>
#include <stdint.h>
#define HEAP_BASE 0xffff900000000000ULL
#define HEAP_LIMIT (64ULL * 1024ULL * 1024ULL)
#define HEAP_ALIGNMENT 16ULL
void heap_init(void);
void *kmalloc(size_t size);
void kfree(void *pointer);
uint64_t heap_used(void);
uint64_t heap_free(void);
#endif
