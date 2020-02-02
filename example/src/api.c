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

// redirect to bootloader USB FS handler
/*void otg_fs_isr(void) {
    __asm volatile(
        //"b #0x080001ad\n"
        //"push {r3}\n"
        "movw r6, #0x0800\n"  // 0x800 - 4 (USB_FS_HANDLER)
        "movt r6, #0x2000\n"
        "ldr pc,[r6,#-4]\n"
    );
}*/

//void application_sys_tick_template(void) { __asm volatile ("nop"); }
//void (*application_sys_tick_handler)(void) = (void (*)(void))application_sys_tick_template;

// do local and redirect to bootloader systick address
/*void sys_tick_handler_broker(void) {
    //application_sys_tick_handler();
    __asm volatile(
        //"b #0x080001b9\n"
        //"pop {r3, lr}\n"
        "movw r6, #0x0800\n"  // 0x800 - 8 (SYSTICK_HANDLER)
        "movt r6, #0x2000\n"
        "ldr pc,[r6,#-8]\n"
    );
}*/

//void application_sys_tick_handler(void);

//void sys_tick_handler(void) {
//    sys_tick_handler_broker();
//}

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
