#include "kernel.h"
#include <stdint.h>

#define ATA_DATA     0x1F0
#define ATA_SECCOUNT 0x1F2
#define ATA_LBA_LOW  0x1F3
#define ATA_LBA_MID  0x1F4
#define ATA_LBA_HIGH 0x1F5
#define ATA_DRIVE    0x1F6
#define ATA_COMMAND  0x1F7
#define ATA_STATUS   0x1F7

static void ata_wait(void) {
    while (inb(ATA_STATUS) & 0x80);
}

void disk_init(void) {
    print("[Disk] ATA initialized (QEMU fs.img attached)\n", 0x0B);
}

void disk_write_sector(int lba, const uint8_t *data) {
    ata_wait();
    outb(ATA_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_SECCOUNT, 1);
    outb(ATA_LBA_LOW, (uint8_t)lba);
    outb(ATA_LBA_MID, (uint8_t)(lba >> 8));
    outb(ATA_LBA_HIGH, (uint8_t)(lba >> 16));
    outb(ATA_COMMAND, 0x30);
    ata_wait();
    outsw(ATA_DATA, data, SECTOR_SIZE / 2);
}

void disk_read_sector(int lba, uint8_t *data) {
    ata_wait();
    outb(ATA_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_SECCOUNT, 1);
    outb(ATA_LBA_LOW, (uint8_t)lba);
    outb(ATA_LBA_MID, (uint8_t)(lba >> 8));
    outb(ATA_LBA_HIGH, (uint8_t)(lba >> 16));
    outb(ATA_COMMAND, 0x20);
    ata_wait();
    insw(ATA_DATA, data, SECTOR_SIZE / 2);
}
