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

#include <libopencm3/cm3/nvic.h>

// redirect to bootloader USB FS handler
void otg_fs_isr(void) {
    /*volatile uint32_t jumpAddress = *(volatile uint32_t*)(USB_FS_HANDLER);*/
    __asm volatile(
        "movw r1, #0x01ad\n"
        "movt r1, #0x0800\n"
        "mov pc, r1\n"
    );
    /*__asm volatile(
        "ldr r1, =0x080001ad\n"
        "mov pc, r1\n"
    );*/
    /*__asm volatile(
        "mov r1, %0\n"
        "mov pc, r1\n"
        :
        : "r"(jumpAddress));*/
}

// do local and redirect to bootloader systick address
void sys_tick_handler(void) {
    //application_sys_tick_handler();
    //volatile uint32_t jumpAddress = *(volatile uint32_t*)(SYSTICK_HANDLER);
    __asm volatile(
        "movw r1, #0x01b9\n"
        "movt r1, #0x0800\n"
        "mov pc, r1\n"
    );
}
