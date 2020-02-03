/*
 * This file is a part of Iskra JS Arduino SDK.
 *
 * Product page: https://amperka.ru/product/iskra-js
 * © Amperka LLC (https://amperka.com, dev@amperka.com)
 * 
 * Author: Yury Botov <by@amperka.com>
 * License: GPLv3, all text here must be included in any redistribution.
 */

#include "api.h"
#include "iskrajs.h"

#include <libopencm3/cm3/cortex.h>
#include <libopencm3/cm3/nvic.h>

// serial USB CDC ACM access functions
void (*syncSerial)(void);
void (*putSerial)(uint8_t);
int16_t (*getSerial)(void);
bool (*availableSerial)(void);
bool (*readyToSendSerial)(void);

#define FUNCTION_ADDRESS(res,par,addr) (res (*)(par))(*((uint32_t*)(addr)))

// serial USB CDC ACM initialisation
void initSerial(void) {
    syncSerial = FUNCTION_ADDRESS(void, void, APPLICATION_RAM-12);
    getSerial = FUNCTION_ADDRESS(int16_t, void, APPLICATION_RAM-16);
    putSerial = FUNCTION_ADDRESS(void, uint8_t, APPLICATION_RAM-20);
    availableSerial = FUNCTION_ADDRESS(bool, void, APPLICATION_RAM-24);
    readyToSendSerial = FUNCTION_ADDRESS(bool, void, APPLICATION_RAM-28);
}
