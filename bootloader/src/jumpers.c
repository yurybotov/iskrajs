#include <libopencm3/cm3/cortex.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/usb/usbd.h>

#include "external.h"
#include "jumpers.h"

extern usbd_device* usbdDevice;

// flag: "started next block writing" - increase relaxLoop duration
volatile bool otherBlock = false;

// flag for bloking create System Volume Information
volatile bool writeEnable = false;

void appJumper(void) {

    cm_disable_interrupts();

    // ?? А НЕ ЛУЧШЕ ЛИ ПЕРЕНЕСТИ ЭТОТ БРЕД В НАЧАЛО ПРИЛОЖЕНИЯ
    // ?? (ПРИ ПРЕДКОМПИЛИРОВАННОМ ВЕКТОРЕ USB FS НАДО ТОЛЬКО VTOR ПЕРЕДВИНУТЬ)

    // Move application interrupt table to RAM
    // Move usb-fs from bootloader to RAM interrupt table
    // Set the interrupt vector to the start of the real image
    //SCB_VTOR = APP_START;

    // safe interface addresses for using in application ?

    // write to RAM address of USB FS handler
    //*((volatile uint32_t*)(RAM_START-4)) = (uint32_t)otg_fs_isr;
    // write to RAM address of USB_SERIAL_RX
    *((volatile uint32_t*)(RAM_START - 8)) = (uint32_t)cdcacm_getc;
    // write to RAM address of USB_SERIAL_TX
    *((volatile uint32_t*)(RAM_START - 12)) = (uint32_t)cdcacm_putc;
    // write to RAM address of USB_SERIAL_AVAILABLE
    *((volatile uint32_t*)(RAM_START - 16)) = (uint32_t)cdcacm_in_ready;
    // write to RAM address of USB_SERIAL_READY
    *((volatile uint32_t*)(RAM_START - 20)) = (uint32_t)cdcacm_out_ready;

    volatile uint32_t stackPtr = *(volatile uint32_t*)(APP_START);
    volatile uint32_t jumpAddress = *(volatile uint32_t*)(APP_START + 4);
    // reset all core registers
    __asm volatile("eor r0, r0\n"
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
    // do some blinking when downloading firmware
#ifdef STM32F407VGT6
    gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT,
        GPIO_PUPD_NONE, GPIO12 | GPIO13 | GPIO14 | GPIO15);
    gpio_set(GPIOD, GPIO12 | GPIO14);
    for (int j = 0; j < 15; j++) {
        if (otherBlock) {
            otherBlock = false;
            j = 0;
        }
        gpio_toggle(GPIOD, GPIO12 | GPIO13 | GPIO14 | GPIO15);
        for (int i = 0; i < 3000000; i++) {
            __asm__("nop");
        }
    }
#endif
#ifdef STM32F405RGT6
    gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT,
        GPIO_PUPD_NONE, GPIO6 | GPIO7);
    gpio_set(GPIOB, GPIO6);
    for (int j = 0; j < 15; j++) {
        if (otherBlock) {
            otherBlock = false;
            j = 0;
        }
        gpio_toggle(GPIOB, GPIO6 | GPIO7);
        for (int i = 0; i < 3000000; i++) {
            __asm__("nop");
        }
    }
#endif
#ifdef STM32F411RG
    gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT,
        GPIO_PUPD_NONE, GPIO2 | GPIO12);
    gpio_set(GPIOB, GPIO2);
    for (int j = 0; j < 15; j++) {
        if (otherBlock) {
            otherBlock = false;
            j = 0;
        }
        gpio_toggle(GPIOB, GPIO2 | GPIO12);
        for (int i = 0; i < 3000000; i++) {
            __asm__("nop");
        }
    }
#endif
    // after blink - reboot
    resetJumper();
}

void relaxJumper(void) {
    cm_disable_interrupts();
    // relocate interrupt vectors
    SCB_VTOR = 0x08000000;
    // replace return address of usb-fs interrupt to relaxLoop
    __asm volatile("mov r1, %0\n"
                   "str r1, [sp,#96]\n" // return address position in stack
                   :
                   : "r"(relaxLoop));
    cm_enable_interrupts();
}
