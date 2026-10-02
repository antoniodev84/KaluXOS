#include "pmm.h"
#include "panic.h"
#include <limine.h>
#include <stdint.h>

#define PMM_PAGE_COUNT (PMM_PHYSICAL_LIMIT / PMM_PAGE_SIZE)
#define PMM_BITMAP_BYTES (PMM_PAGE_COUNT / 8)

static uint8_t bitmap[PMM_BITMAP_BYTES];
static uint64_t total_pages;
static uint64_t free_pages;
static uint64_t scan_hint;

static volatile struct limine_memmap_request memmap_request __attribute__((used, section(".limine_requests"))) = {
    .id = LIMINE_MEMMAP_REQUEST_ID,
    .revision = 0,
    .response = 0
};

static uint64_t page_index(uint64_t address) {
    return address / PMM_PAGE_SIZE;
}

static void mark_used(uint64_t index) {
    uint8_t mask = (uint8_t)(1U << (index & 7));
    if ((bitmap[index >> 3] & mask) == 0) {
        bitmap[index >> 3] |= mask;
        if (free_pages != 0) {
            free_pages--;
        }
    }
}

static void mark_free(uint64_t index) {
    uint8_t mask = (uint8_t)(1U << (index & 7));
    if ((bitmap[index >> 3] & mask) != 0) {
        bitmap[index >> 3] &= (uint8_t)~mask;
        free_pages++;
    }
}

void pmm_init(void) {
    if (memmap_request.response == 0 || memmap_request.response->entry_count == 0) {
        panic("PMM: memory map unavailable");
    }
    for (uint64_t i = 0; i < PMM_BITMAP_BYTES; i++) {
        bitmap[i] = 0xff;
    }
    total_pages = PMM_PAGE_COUNT;
    free_pages = 0;
    scan_hint = 0;
    for (uint64_t i = 0; i < memmap_request.response->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap_request.response->entries[i];
        if (entry->type != LIMINE_MEMMAP_USABLE || entry->base >= PMM_PHYSICAL_LIMIT) {
            continue;
        }
        uint64_t end = entry->base + entry->length;
        if (end < entry->base || end > PMM_PHYSICAL_LIMIT) {
            end = PMM_PHYSICAL_LIMIT;
        }
        uint64_t first = (entry->base + PMM_PAGE_SIZE - 1) / PMM_PAGE_SIZE;
        uint64_t last = end / PMM_PAGE_SIZE;
        for (uint64_t page = first; page < last; page++) {
            mark_free(page);
        }
    }
    uint64_t bitmap_start = (uint64_t)(uintptr_t)bitmap;
    uint64_t bitmap_end = bitmap_start + PMM_BITMAP_BYTES;
    if (bitmap_start < PMM_PHYSICAL_LIMIT) {
        uint64_t first = page_index(bitmap_start);
        uint64_t last = (bitmap_end + PMM_PAGE_SIZE - 1) / PMM_PAGE_SIZE;
        for (uint64_t page = first; page < last && page < PMM_PAGE_COUNT; page++) {
            mark_used(page);
        }
    }
    if (free_pages == 0) {
        panic("PMM: no usable pages");
    }
}

uint64_t pmm_alloc_page(void) {
    for (uint64_t n = 0; n < total_pages; n++) {
        uint64_t index = (scan_hint + n) % total_pages;
        if ((bitmap[index >> 3] & (1U << (index & 7))) == 0) {
            mark_used(index);
            scan_hint = (index + 1) % total_pages;
            return index * PMM_PAGE_SIZE;
        }
    }
    return 0;
}

void pmm_free_page(uint64_t address) {
    if ((address & (PMM_PAGE_SIZE - 1)) != 0 || address >= PMM_PHYSICAL_LIMIT) {
        return;
    }
    mark_free(page_index(address));
    if (page_index(address) < scan_hint) {
        scan_hint = page_index(address);
    }
}

uint64_t pmm_total_pages(void) {
    return total_pages;
}

uint64_t pmm_free_pages(void) {
    return free_pages;
}
