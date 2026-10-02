#include "vmm.h"
#include "pmm.h"
#include "panic.h"
#include "memory.h"
#include <limine.h>
#include <stdint.h>

static volatile struct limine_hhdm_request hhdm_request __attribute__((used, section(".limine_requests"))) = {
    .id = LIMINE_HHDM_REQUEST_ID,
    .revision = 0,
    .response = 0
};

static uint64_t hhdm_offset;
static uint64_t root_physical;

static uint64_t read_cr3(void) {
    uint64_t value;
    __asm__ volatile ("mov %%cr3, %0" : "=r"(value));
    return value & ~0xfffULL;
}

static void invalidate(uint64_t address) {
    __asm__ volatile ("invlpg (%0)" : : "r"(address) : "memory");
}

void *vmm_phys_to_virt(uint64_t address) {
    return (void *)(uintptr_t)(address + hhdm_offset);
}

static uint64_t *table(uint64_t physical) {
    return (uint64_t *)vmm_phys_to_virt(physical);
}

static uint64_t new_table(void) {
    uint64_t physical = pmm_alloc_page();
    if (physical == 0) {
        return 0;
    }
    memset(vmm_phys_to_virt(physical), 0, VMM_PAGE_SIZE);
    return physical;
}

static uint64_t ensure_table(uint64_t *entry) {
    if ((*entry & VMM_PRESENT) != 0) {
        return *entry & ~0xfffULL;
    }
    uint64_t physical = new_table();
    if (physical == 0) {
        return 0;
    }
    *entry = physical | VMM_PRESENT | VMM_WRITABLE;
    return physical;
}

void vmm_init(void) {
    if (hhdm_request.response == 0) {
        panic("VMM: HHDM unavailable");
    }
    hhdm_offset = hhdm_request.response->offset;
    root_physical = read_cr3();
    if (root_physical == 0) {
        panic("VMM: active page table unavailable");
    }
}

uint64_t vmm_hhdm(void) {
    return hhdm_offset;
}

int vmm_map(uint64_t virtual_address, uint64_t physical_address, uint64_t flags) {
    if ((virtual_address & (VMM_PAGE_SIZE - 1)) != 0 || (physical_address & (VMM_PAGE_SIZE - 1)) != 0) {
        return -1;
    }
    uint64_t *pml4 = table(root_physical);
    uint64_t pml4_entry = (virtual_address >> 39) & 0x1ff;
    uint64_t pdpt_physical = ensure_table(&pml4[pml4_entry]);
    if (pdpt_physical == 0) {
        return -1;
    }
    uint64_t *pdpt = table(pdpt_physical);
    uint64_t pdpt_entry = (virtual_address >> 30) & 0x1ff;
    uint64_t pd_physical = ensure_table(&pdpt[pdpt_entry]);
    if (pd_physical == 0) {
        return -1;
    }
    uint64_t *pd = table(pd_physical);
    uint64_t pd_entry = (virtual_address >> 21) & 0x1ff;
    uint64_t pt_physical = ensure_table(&pd[pd_entry]);
    if (pt_physical == 0) {
        return -1;
    }
    uint64_t *pt = table(pt_physical);
    uint64_t pt_entry = (virtual_address >> 12) & 0x1ff;
    if ((pt[pt_entry] & VMM_PRESENT) != 0) {
        return -1;
    }
    pt[pt_entry] = (physical_address & ~0xfffULL) | flags | VMM_PRESENT;
    invalidate(virtual_address);
    return 0;
}

int vmm_unmap(uint64_t virtual_address, uint64_t *physical_address) {
    if ((virtual_address & (VMM_PAGE_SIZE - 1)) != 0) {
        return -1;
    }
    uint64_t *pml4 = table(root_physical);
    uint64_t pml4_entry = (virtual_address >> 39) & 0x1ff;
    if ((pml4[pml4_entry] & VMM_PRESENT) == 0) {
        return -1;
    }
    uint64_t *pdpt = table(pml4[pml4_entry] & ~0xfffULL);
    uint64_t pdpt_entry = (virtual_address >> 30) & 0x1ff;
    if ((pdpt[pdpt_entry] & VMM_PRESENT) == 0) {
        return -1;
    }
    uint64_t *pd = table(pdpt[pdpt_entry] & ~0xfffULL);
    uint64_t pd_entry = (virtual_address >> 21) & 0x1ff;
    if ((pd[pd_entry] & VMM_PRESENT) == 0) {
        return -1;
    }
    uint64_t *pt = table(pd[pd_entry] & ~0xfffULL);
    uint64_t pt_entry = (virtual_address >> 12) & 0x1ff;
    if ((pt[pt_entry] & VMM_PRESENT) == 0) {
        return -1;
    }
    if (physical_address != 0) {
        *physical_address = pt[pt_entry] & ~0xfffULL;
    }
    pt[pt_entry] = 0;
    invalidate(virtual_address);
    return 0;
}
