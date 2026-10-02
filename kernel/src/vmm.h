#ifndef KALUXOS_VMM_H
#define KALUXOS_VMM_H
#include <stdint.h>
#define VMM_PAGE_SIZE 4096ULL
#define VMM_PRESENT 0x001ULL
#define VMM_WRITABLE 0x002ULL
#define VMM_USER 0x004ULL
#define VMM_NO_EXECUTE (1ULL << 63)
void vmm_init(void);
uint64_t vmm_hhdm(void);
void *vmm_phys_to_virt(uint64_t address);
int vmm_map(uint64_t virtual_address, uint64_t physical_address, uint64_t flags);
int vmm_unmap(uint64_t virtual_address, uint64_t *physical_address);
#endif
