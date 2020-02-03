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

void (*syncSerial)(void);
void (*putSerial)(uint8_t);
int16_t (*getSerial)(void);
bool (*availableSerial)(void);
bool (*readyToSendSerial)(void);

void initSerial(void) {
    syncSerial = (void (*)(void))(*((uint32_t*)(APPLICATION_RAM-12)));
    getSerial = (int16_t (*)(void))(*((uint32_t*)(APPLICATION_RAM-16)));
    putSerial = (void (*)(uint8_t))(*((uint32_t*)(APPLICATION_RAM-20)));
    availableSerial = (bool (*)(void))(*((uint32_t*)(APPLICATION_RAM-24)));
    readyToSendSerial = (bool (*)(void))(*((uint32_t*)(APPLICATION_RAM-28)));
}
