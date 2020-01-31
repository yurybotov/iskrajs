/*
 * This file is a part of Iskra JS Arduino SDK.
 *
 * Product page: https://amperka.ru/product/iskra-js
 * © Amperka LLC (https://amperka.com, dev@amperka.com)
 * 
 * Author: Yury Botov <by@amperka.com>
 * License: GPLv3, all text here must be included in any redistribution.
 *
 * FYI: button - if user firmware has crash, you can reload right firmware:
 * press and hold User button, and press Reset button for 1 sec and release it,
 * after that release User button. Bootloader ready to load new firmware
 */

#include <stdlib.h>

#include <libopencm3/cm3/cortex.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/usb/usbd.h>

#include "systemtick.h"
#include "jumpers.h"
#include "romdisk.h"
#include "cdc.h"
#include "serialcore.h"

#include "depend.h"

extern usbd_device* usbdDevice;
extern const struct usb_device_descriptor dev_descr;
extern const struct usb_config_descriptor config_descr;
extern const char* usb_strings[];

extern uint32_t magic_point;

#include "usb.h"

// on usb-fs interrupt
void otg_fs_isr(void) { usbd_poll(usbdDevice); }

// on systick interrupt
extern volatile uint32_t system_millis;
void sys_tick_handler(void) { 
    if(++system_millis == 100) { // 0.1 sec
        system_millis = 0;
        cdcacm_sync();
    }
}

static uint8_t usbd_control_buffer[128];

int main(void) {
    // clock initialisation 168MHz on 8MHz quartz (405/407)
    rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ]);

    // power-on usb-fs, leds and button pins
    rcc_periph_clock_enable(RCC_OTGFS);
    initPins();

    // Serial USB CDC ACM init
    cdcacm_init();

    // FAT12 file system on pceudo drive init
    romdisk_init();

    // usb-fs initialisation
    usbdDevice = usbd_init(&otgfs_usb_driver, &dev_descr, &config_descr, usb_strings, 3,
        usbd_control_buffer, sizeof(usbd_control_buffer));

    // msc initialisation
    usb_msc_init(usbdDevice, 0x81, 64, 0x01, 64, "Amperka", "IskraJS", "1.00.00",
        romdisk_blocks(), romdisk_read, romdisk_write);
    // cdc handler connection
    usbd_register_set_config_callback(usbdDevice, cdcacm_set_config);

    // usb-fs interrupt enabling
    nvic_set_priority(NVIC_OTG_FS_IRQ, 2 << 4);
    nvic_enable_irq(NVIC_OTG_FS_IRQ);
    rcc_peripheral_enable_clock(&RCC_AHB2ENR, 1 << 7);

    // if button pressed - do not start application, only bootloader (emergency mode)
    // otherwise - run application after bootloader init (normal mode)
    if (buttonReleased()) {
        // init systick as 1 kHz
        systick_setup();        
        appJumper();
    } else {
        while (1)
            __asm__("nop");
    }
}
