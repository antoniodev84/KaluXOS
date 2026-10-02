#ifndef KALUXOS_BLOCK_H
#define KALUXOS_BLOCK_H
#include <stdint.h>
#define BLOCK_SECTOR_SIZE 512U
typedef struct block_device {
    uint64_t sector_count;
    int (*read)(uint64_t sector, void *buffer);
    int (*write)(uint64_t sector, const void *buffer);
} block_device_t;
void block_init(void);
block_device_t *block_root(void);
int block_read(uint64_t sector, void *buffer);
int block_write(uint64_t sector, const void *buffer);
#endif
