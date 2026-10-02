#include "heap.h"
#include "pmm.h"
#include "vmm.h"
#include "panic.h"
#include "memory.h"
#include <stdint.h>

#define HEAP_MAGIC 0x4b58534845415031ULL

typedef struct heap_block {
    uint64_t magic;
    uint64_t size;
    uint8_t free;
    uint8_t reserved[7];
    struct heap_block *next;
    struct heap_block *previous;
} heap_block_t;

static heap_block_t *first_block;
static uint64_t used_bytes;
static uint64_t free_bytes;

static uint64_t align_size(uint64_t size) {
    return (size + HEAP_ALIGNMENT - 1) & ~(HEAP_ALIGNMENT - 1);
}

static void split_block(heap_block_t *block, uint64_t size) {
    uint64_t remaining = block->size - size;
    if (remaining < sizeof(heap_block_t) + HEAP_ALIGNMENT) {
        return;
    }
    heap_block_t *next = (heap_block_t *)((uint8_t *)(block + 1) + size);
    next->magic = HEAP_MAGIC;
    next->size = remaining - sizeof(heap_block_t);
    next->free = 1;
    next->next = block->next;
    next->previous = block;
    if (next->next != 0) {
        next->next->previous = next;
    }
    block->next = next;
    block->size = size;
    free_bytes -= sizeof(heap_block_t);
}

void heap_init(void) {
    uint64_t pages = HEAP_LIMIT / VMM_PAGE_SIZE;
    for (uint64_t i = 0; i < pages; i++) {
        uint64_t physical = pmm_alloc_page();
        if (physical == 0 || vmm_map(HEAP_BASE + i * VMM_PAGE_SIZE, physical, VMM_WRITABLE | VMM_NO_EXECUTE) != 0) {
            panic("HEAP: virtual space unavailable");
        }
    }
    first_block = (heap_block_t *)(uintptr_t)HEAP_BASE;
    first_block->magic = HEAP_MAGIC;
    first_block->size = HEAP_LIMIT - sizeof(heap_block_t);
    first_block->free = 1;
    first_block->next = 0;
    first_block->previous = 0;
    used_bytes = 0;
    free_bytes = first_block->size;
}

void *kmalloc(size_t size) {
    if (size == 0) {
        return 0;
    }
    uint64_t wanted = align_size((uint64_t)size);
    for (heap_block_t *block = first_block; block != 0; block = block->next) {
        if (block->free != 0 && block->size >= wanted) {
            split_block(block, wanted);
            block->free = 0;
            used_bytes += block->size;
            free_bytes -= block->size;
            return (void *)(block + 1);
        }
    }
    return 0;
}

static void merge_next(heap_block_t *block) {
    heap_block_t *next = block->next;
    if (next == 0 || next->free == 0) {
        return;
    }
    block->size += sizeof(heap_block_t) + next->size;
    block->next = next->next;
    if (block->next != 0) {
        block->next->previous = block;
    }
    free_bytes += sizeof(heap_block_t);
}

void kfree(void *pointer) {
    if (pointer == 0) {
        return;
    }
    uintptr_t address = (uintptr_t)pointer;
    if (address < HEAP_BASE + sizeof(heap_block_t) || address >= HEAP_BASE + HEAP_LIMIT) {
        return;
    }
    heap_block_t *block = ((heap_block_t *)pointer) - 1;
    if (block->magic != HEAP_MAGIC || block->free != 0) {
        return;
    }
    block->free = 1;
    used_bytes -= block->size;
    free_bytes += block->size;
    if (block->next != 0 && block->next->free != 0) {
        merge_next(block);
    }
    if (block->previous != 0 && block->previous->free != 0) {
        merge_next(block->previous);
    }
}

uint64_t heap_used(void) {
    return used_bytes;
}

uint64_t heap_free(void) {
    return free_bytes;
}
