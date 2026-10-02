#ifndef KALUXOS_PMM_H
#define KALUXOS_PMM_H
#include <stdint.h>
#define PMM_PAGE_SIZE 4096ULL
#define PMM_PHYSICAL_LIMIT (4ULL * 1024ULL * 1024ULL * 1024ULL)
void pmm_init(void);
uint64_t pmm_alloc_page(void);
void pmm_free_page(uint64_t address);
uint64_t pmm_total_pages(void);
uint64_t pmm_free_pages(void);
#endif
