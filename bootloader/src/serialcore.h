/*
 * This file is a part of Iskra JS Arduino SDK.
 *
 * Product page: https://amperka.ru/product/iskra-js
 * © Amperka LLC (https://amperka.com, dev@amperka.com)
 * 
 * Author: Yury Botov <by@amperka.com>
 * License: GPLv3, all text here must be included in any redistribution.
 */

#ifndef __SERIAL_CORE_H__
#define __SERIAL_CORE_H__

#include <libopencm3/usb/usbd.h>

void cdcacm_sync(void);

void cdcacm_data_tx_all(usbd_device* usbd_dev);

void cdcacm_init(void);

bool cdcacm_out_ready(void);

void cdcacm_putc(uint8_t c);

bool cdcacm_in_ready(void);

int cdcacm_getc(void);

#endif
