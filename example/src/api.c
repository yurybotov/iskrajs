/*
 * This file is a part of Iskra JS Arduino SDK.
 *
 * Product page: https://amperka.ru/product/iskra-js
 * © Amperka LLC (https://amperka.com, dev@amperka.com)
 * 
 * Author: Yury Botov <by@amperka.com>
 * License: GPLv3, all text here must be included in any redistribution.
 */

#include <stdint.h>

#include "api.h"
#include "iskrajs.h"

#include <libopencm3/cm3/cortex.h>
#include <libopencm3/cm3/nvic.h>

// redirect to bootloader USB FS handler
void otg_fs_isr(void) {
    __asm volatile(
        //"b #0x080001ad\n"
        "movw r1, #0x0800\n"  // 0x800 - 4 (USB_FS_HANDLER)
        "movt r1, #0x2000\n"
        "ldr r1,[r1,#-4]\n"
        "mov pc, r1\n"
        );
}

// do local and redirect to bootloader systick address
void sys_tick_handler(void) {
    //application_sys_tick_handler();
    __asm volatile(
        //"b #0x080001b9\n"
        "movw r1, #0x0800\n"  // 0x800 - 8 (SYSTICK_HANDLER)
        "movt r1, #0x2000\n"
        "ldr r1,[r1,#-8]\n"
        "mov pc, r1\n"
        );
}
