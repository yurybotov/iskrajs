/*
 * This file is a part of Iskra JS Arduino SDK.
 *
 * Product page: https://amperka.ru/product/iskra-js
 * © Amperka LLC (https://amperka.com, dev@amperka.com)
 * 
 * Author: Yury Botov <by@amperka.com>
 * License: GPLv3, all text here must be included in any redistribution.
 */

#include <libopencm3/cm3/cortex.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/usb/usbd.h>

#include "serialcore.h"
#include "systemtick.h"
#include "jumpers.h"

#include "depend.h"

extern usbd_device* usbdDevice;
//extern void otg_fs_isr(void);

// flag: "started next block writing" - increase relaxLoop duration
volatile bool otherBlock = false;

// flag for bloking create System Volume Information
volatile bool writeEnable = false;

void appJumper(void) {
    // write to RAM address of USB FS handler
    *((volatile uint32_t*)(RAM_APP_START - 4)) = (uint32_t)otg_fs_isr;
    // write to RAM address of SysTick handler
    *((volatile uint32_t*)(RAM_APP_START - 8)) = (uint32_t)sys_tick_handler;
    // write to RAM address of USB_SERIAL_SEND
    *((volatile uint32_t*)(RAM_APP_START - 12)) = (uint32_t)cdcacm_sync;
    // write to RAM address of USB_SERIAL_RX
    *((volatile uint32_t*)(RAM_APP_START - 16)) = (uint32_t)cdcacm_getc;
    // write to RAM address of USB_SERIAL_TX
    *((volatile uint32_t*)(RAM_APP_START - 20)) = (uint32_t)cdcacm_putc;
    // write to RAM address of USB_SERIAL_AVAILABLE
    *((volatile uint32_t*)(RAM_APP_START - 24)) = (uint32_t)cdcacm_in_ready;
    // write to RAM address of USB_SERIAL_READY
    *((volatile uint32_t*)(RAM_APP_START - 28)) = (uint32_t)cdcacm_out_ready;

    cm_disable_interrupts();

    volatile uint32_t stackPtr = *(volatile uint32_t*)(ROM_APP_START);
    volatile uint32_t jumpAddress = *(volatile uint32_t*)(ROM_APP_START + 4);
    // reset core registers set stack and jump to address
    __asm volatile(
        "eor r0, r0\n"
        "mov r1, %0\n"
        "mov r2, %1\n"
        "msr psp, r0\n"
        "msr msp, r2\n"
        "msr ipsr, r0\n"
        "msr primask, r0\n"
        "msr faultmask, r0\n"
        "msr basepri, r0\n"
        "msr control, r0\n"
        "eor lr, lr\n"
        "sub lr, #1\n"
        "mov pc, r1\n"
        :
        : "r"(jumpAddress), "r"(stackPtr));
}

void resetJumper(void) {
    usbd_disconnect(usbdDevice, true);
    scb_reset_system();
}

static void relaxLoop(void) {
    // relocate interrupt vectors
    cm_disable_interrupts();
    SCB_VTOR = 0x08000000;
    cm_enable_interrupts();
    // do some blinking when downloading firmware
    showLeds();
    for (int j = 0; j < 15; j++) {
        if (otherBlock) {
            otherBlock = false;
            j = 0;
        }
        toggleLeds();
        for (int i = 0; i < 3000000; i++) {
            __asm__("nop");
        }
    }
    hideLeds();
    // after blink - reboot
    resetJumper();
}

void relaxJumper(void) {
    //systick_block();
    cm_disable_interrupts();
    // relocate interrupt vectors
    SCB_VTOR = 0x08000000;
    // replace return address of usb-fs interrupt to relaxLoop
    __asm volatile(
        "mov r1, %0\n"
        "str r1, [sp,#96]\n" // return address position in stack
        :
        : "r"(relaxLoop));
    cm_enable_interrupts();
}
