/*
 * This file is a part of Iskra JS Arduino SDK.
 *
 * Product page: https://amperka.ru/product/iskra-js
 * © Amperka LLC (https://amperka.com, dev@amperka.com)
 * 
 * Author: Yury Botov <by@amperka.com>
 * License: GPLv3, all text here must be included in any redistribution.
 */

#include "iskrajs.h"

#include <libopencm3/cm3/cortex.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

#include "depend.h"

#define DELAY 50000000

int main(void) {
    SCB_VTOR = 0x08004000;
    showLeds();
    while (1) {
        toggleLeds();
        for (int i = 0; i < DELAY; i++) {
            __asm__("nop");
        }
    }
    cm_enable_interrupts();
    return 0;
}
