#ifndef __ROMDISK_H
#define __ROMDISK_H

#include <stdint.h>

int romdisk_init(void);
int romdisk_read(uint32_t lba, uint8_t* copy_to);
int romdisk_write(uint32_t lba, const uint8_t* copy_from);
int romdisk_blocks(void);

#endif
