/*
 * This file is a part of Iskra JS Arduino SDK.
 *
 * Product page: https://amperka.ru/product/iskra-js
 * © Amperka LLC (https://amperka.com, dev@amperka.com)
 * 
 * Author: Yury Botov <by@amperka.com>
 * License: GPLv3, all text here must be included in any redistribution.
 */

#ifndef __EXTERNAL_H__
#define __EXTERNAL_H__

#include <stdint.h>
#include <stdbool.h>

// void otg_fs_isr(void); // defined at ../libopencm3/include/libopencm3/stm32/f4/nvic.h

bool cdcacm_out_ready(void);

void cdcacm_putc(uint8_t c);

bool cdcacm_in_ready(void);

int cdcacm_getc(void);

#endif
