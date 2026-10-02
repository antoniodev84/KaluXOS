#include "block.h"
#include "uart.h"
#include <stdint.h>
#define ATA_DATA 0x1f0
#define ATA_ERROR 0x1f1
#define ATA_SECCOUNT 0x1f2
#define ATA_LBA0 0x1f3
#define ATA_LBA1 0x1f4
#define ATA_LBA2 0x1f5
#define ATA_HDDEVSEL 0x1f6
#define ATA_STATUS 0x1f7
#define ATA_COMMAND 0x1f7
#define ATA_CONTROL 0x3f6
#define ATA_CMD_READ 0x20
#define ATA_CMD_WRITE 0x30
#define ATA_CMD_IDENTIFY 0xec
#define ATA_SR_BSY 0x80
#define ATA_SR_DRDY 0x40
#define ATA_SR_DRQ 0x08
#define ATA_SR_ERR 0x01
#define ATA_TIMEOUT 1000000U
static block_device_t device;
static int present;
static inline void outb(uint16_t port, uint8_t value) { __asm__ volatile ("outb %0,%1" : : "a"(value), "Nd"(port)); }
static inline uint8_t inb(uint16_t port) { uint8_t value; __asm__ volatile ("inb %1,%0" : "=a"(value) : "Nd"(port)); return value; }
static inline void outw(uint16_t port, uint16_t value) { __asm__ volatile ("outw %0,%1" : : "a"(value), "Nd"(port)); }
static inline uint16_t inw(uint16_t port) { uint16_t value; __asm__ volatile ("inw %1,%0" : "=a"(value) : "Nd"(port)); return value; }
static void io_delay(void) { inb(ATA_CONTROL); inb(ATA_CONTROL); inb(ATA_CONTROL); inb(ATA_CONTROL); }
static int wait(uint8_t mask, uint8_t value) { for (uint32_t i = 0; i < ATA_TIMEOUT; i++) { uint8_t status = inb(ATA_STATUS); if ((status & ATA_SR_ERR) != 0) return -1; if ((status & mask) == value) return 0; } return -1; }
static int ata_rw(uint64_t sector, void *buffer, int write) {
    if (!present || sector >= device.sector_count || buffer == 0) return -1;
    if (sector > 0x0fffffffULL) return -1;
    if (wait(ATA_SR_BSY, 0) != 0) return -1;
    outb(ATA_HDDEVSEL, 0xe0 | ((sector >> 24) & 0x0f));
    io_delay();
    outb(ATA_SECCOUNT, 1);
    outb(ATA_LBA0, sector & 0xff);
    outb(ATA_LBA1, (sector >> 8) & 0xff);
    outb(ATA_LBA2, (sector >> 16) & 0xff);
    outb(ATA_COMMAND, write ? ATA_CMD_WRITE : ATA_CMD_READ);
    if (wait(ATA_SR_BSY | ATA_SR_DRQ, ATA_SR_DRQ) != 0) return -1;
    uint16_t *words = (uint16_t *)buffer;
    if (write) { for (uint32_t i = 0; i < 256; i++) outw(ATA_DATA, words[i]); } else { for (uint32_t i = 0; i < 256; i++) words[i] = inw(ATA_DATA); }
    if (write) { outb(ATA_COMMAND, 0xe7); if (wait(ATA_SR_BSY, 0) != 0) return -1; }
    return 0;
}
static int ata_read(uint64_t sector, void *buffer) { return ata_rw(sector, buffer, 0); }
static int ata_write(uint64_t sector, const void *buffer) { return ata_rw(sector, (void *)buffer, 1); }
void block_init(void) {
    present = 0;
    outb(ATA_HDDEVSEL, 0xa0);
    io_delay();
    outb(ATA_SECCOUNT, 0);
    outb(ATA_LBA0, 0);
    outb(ATA_LBA1, 0);
    outb(ATA_LBA2, 0);
    outb(ATA_COMMAND, ATA_CMD_IDENTIFY);
    if (inb(ATA_STATUS) == 0 || wait(ATA_SR_BSY, 0) != 0 || (inb(ATA_STATUS) & ATA_SR_DRQ) == 0) { uart_puts("[ ERR ] ATA disk unavailable\n"); return; }
    uint16_t identify[256];
    for (uint32_t i = 0; i < 256; i++) identify[i] = inw(ATA_DATA);
    uint64_t sectors = ((uint64_t)identify[61] << 16) | identify[60];
    if (sectors == 0) { uart_puts("[ ERR ] ATA disk has no sectors\n"); return; }
    device.sector_count = sectors;
    device.read = ata_read;
    device.write = ata_write;
    present = 1;
    uart_puts("[ OK ] ATA PIO disk initialized, sectors: ");
    uart_put_dec(sectors);
    uart_puts("\n");
}
block_device_t *block_root(void) { return present ? &device : 0; }
int block_read(uint64_t sector, void *buffer) { return present ? device.read(sector, buffer) : -1; }
int block_write(uint64_t sector, const void *buffer) { return present ? device.write(sector, buffer) : -1; }
