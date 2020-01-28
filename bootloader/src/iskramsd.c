#include <stdlib.h>

#include <libopencm3/cm3/cortex.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/stm32/gpio.h>

#include "romdisk.h"
#include "external.h"
#include "jumpers.h"

extern usbd_device* usbdDevice;
extern const struct usb_device_descriptor dev_descr;
extern const struct usb_config_descriptor config_descr;
extern const char* usb_strings[];

#include "usb.h"

static uint8_t usbd_control_buffer[128];

extern uint32_t magic_point;

int main(void) {
    // clock initialisation 168MHz on 8MHz quarz (405/407)
    rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ]);

    // power-on usb-fs, leds and button pins
#ifdef STM32F407VGT6
    rcc_periph_clock_enable(RCC_GPIOD); // leds
    rcc_periph_clock_enable(RCC_GPIOA); // button A0 and usb PA11 PA12
#endif
#ifdef STM32F405RGT6
    rcc_periph_clock_enable(RCC_GPIOC); // button C4
    rcc_periph_clock_enable(RCC_GPIOB); // leds B6 B7
    rcc_periph_clock_enable(RCC_GPIOA); // usb PA11 PA12
#endif
#ifdef STM32F411RG
    rcc_periph_clock_enable(RCC_GPIOB); // leds PB2 PB12 button PB1
    rcc_periph_clock_enable(RCC_GPIOA); // usb PA11 PA12
#endif   
    
    // power-on usb-fs
    rcc_periph_clock_enable(RCC_OTGFS);

    // button - if user firmware has crash, you can reload right firmware:
    // press and hold User button, and press Reset button for 1 sec and release it, 
    // after that release User button. Bootloader ready to load new firmware.
#ifdef STM32F407VGT6
    gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, GPIO0);
#endif
#ifdef STM32F405RGT6
    gpio_mode_setup(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, GPIO4);
#endif
#ifdef STM32F411RG
    gpio_mode_setup(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, GPIO1);
#endif

    // connect pins to usb-fs
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO11 | GPIO12);
    gpio_set_af(GPIOA, GPIO_AF10, GPIO11 | GPIO12);

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
#ifdef STM32F407VGT6
    if( !(gpio_port_read(GPIOA) & GPIO0)) {
#endif
#ifdef STM32F405RGT6
    if( !(gpio_port_read(GPIOC) & GPIO4)) {
#endif
#ifdef STM32F411RG
    if( !(gpio_port_read(GPIOB) & GPIO1)) {
#endif
        appJumper();
    } else {
        while(1)
            __asm__("nop");
    }
}

// on usb-fs interrupt
void otg_fs_isr(void) { usbd_poll(usbdDevice); }
