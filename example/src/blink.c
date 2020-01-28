#include "iskrajs.h"

#include <libopencm3/cm3/cortex.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

#define DELAY 5000000

int main(void) {
    SCB_VTOR = 0x08004000;

    cm_enable_interrupts();
#ifdef STM32F407VGT6
    rcc_periph_clock_enable(RCC_GPIOD);
    gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT,
        GPIO_PUPD_NONE, GPIO12 | GPIO13 | GPIO14 | GPIO15);
    gpio_set(GPIOD, GPIO12 | GPIO14);
    while (1) {
        gpio_toggle(GPIOD, GPIO12 | GPIO13 | GPIO14 | GPIO15);
        for (int i = 0; i < DELAY; i++) {
            __asm__("nop");
        }
    }
#endif
#ifdef STM32F405RGT6
    rcc_periph_clock_enable(RCC_GPIOB);
    gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT,
        GPIO_PUPD_NONE, GPIO6 | GPIO7);
    gpio_set(GPIOB, GPIO6);
    while (1) {
        gpio_toggle(GPIOB, GPIO6 | GPIO7);
        for (int i = 0; i < DELAY; i++) {
            __asm__("nop");
        }
    }
#endif
#ifdef STM32F411RG
    rcc_periph_clock_enable(RCC_GPIOB);
    gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT,
        GPIO_PUPD_NONE, GPIO2 | GPIO12);
    gpio_set(GPIOB, GPIO2);
    while (1) {
        gpio_toggle(GPIOB, GPIO2 | GPIO12);
        for (int i = 0; i < DELAY; i++) {
            __asm__("nop");
        }
    }
#endif

    return 0;
}
