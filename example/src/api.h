/*
 * This file is a part of Iskra JS Arduino SDK.
 *
 * Product page: https://amperka.ru/product/iskra-js
 * © Amperka LLC (https://amperka.com, dev@amperka.com)
 * 
 * Author: Yury Botov <by@amperka.com>
 * License: GPLv3, all text here must be included in any redistribution.
 */

#ifndef __API_H__
#define __API_H__

#include <stdbool.h>
#include <stdint.h>

void sys_tick_handler_broker(void);

// you can override this function for sistick access
void application_sys_tick_handler(void) __attribute__((weak));

void initSerial(void);

extern void (*syncSerial)(void);
extern void (*putSerial)(uint8_t);
extern int16_t (*getSerial)(void);
extern bool (*availableSerial)(void);
extern bool (*readyToSendSerial)(void);

#endif
