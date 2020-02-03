/*
 * This file is a part of Iskra JS Arduino SDK.
 *
 * Product page: https://amperka.ru/product/iskra-js
 * © Amperka LLC (https://amperka.com, dev@amperka.com)
 * 
 * Author: Yury Botov <by@amperka.com>
 * License: GPLv3, all text here must be included in any redistribution.
 */

#ifndef __ROMDISK_H
#define __ROMDISK_H

#include <stdint.h>

int romdisk_init(void);
int romdisk_read(uint32_t lba, uint8_t* copy_to);
int romdisk_write(uint32_t lba, const uint8_t* copy_from);
int romdisk_blocks(void);

#endif
