/*
 * This file is a part of Iskra JS Arduino SDK.
 *
 * Product page: https://amperka.ru/product/iskra-js
 * © Amperka LLC (https://amperka.com, dev@amperka.com)
 * 
 * Author: Yury Botov <by@amperka.com>
 * License: GPLv3, all text here must be included in any redistribution.
 */

#include "depend.h"

void initPins() {
#ifdef STM32F407VGT6
    rcc_periph_clock_enable(RCC_GPIOD); // leds
    rcc_periph_clock_enable(RCC_GPIOA); // button A0 and usb PA11 PA12
    gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, GPIO0);   
#endif
#ifdef STM32F405RGT6
    rcc_periph_clock_enable(RCC_GPIOC); // button C4
    rcc_periph_clock_enable(RCC_GPIOB); // leds B6 B7
    rcc_periph_clock_enable(RCC_GPIOA); // usb PA11 PA12
    gpio_mode_setup(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, GPIO4);
#endif
#ifdef STM32F411RG
    rcc_periph_clock_enable(RCC_GPIOB); // leds PB2 PB12 button PB1
    rcc_periph_clock_enable(RCC_GPIOA); // usb PA11 PA12
    gpio_mode_setup(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, GPIO1);
#endif
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO11 | GPIO12); // usb
    gpio_set_af(GPIOA, GPIO_AF10, GPIO11 | GPIO12); 
}

void showLeds() {
#ifdef STM32F407VGT6
    gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO12 | GPIO13 | GPIO14 | GPIO15);
    gpio_set(GPIOD, GPIO12 | GPIO14);
#endif
#ifdef STM32F405RGT6
    gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO6 | GPIO7);
    gpio_set(GPIOB, GPIO6);
#endif
#ifdef STM32F411RG
    gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT,
        GPIO_PUPD_NONE, GPIO2 | GPIO12);
    gpio_set(GPIOB, GPIO2);
#endif
}

void toggleLeds() {
#ifdef STM32F407VGT6
    gpio_toggle(GPIOD, GPIO12 | GPIO13 | GPIO14 | GPIO15);
#endif
#ifdef STM32F405RGT6
    gpio_toggle(GPIOB, GPIO6 | GPIO7);
#endif
#ifdef STM32F411RG
    gpio_toggle(GPIOB, GPIO2 | GPIO12);
#endif
}

void hideLeds() {
#ifdef STM32F407VGT6
    gpio_clear(GPIOD, GPIO12 | GPIO13 | GPIO14 | GPIO15);
#endif
#ifdef STM32F405RGT6
    gpio_clear(GPIOB, GPIO6 | GPIO7);
#endif
#ifdef STM32F411RG
    gpio_clear(GPIOB, GPIO2 | GPIO12);
#endif
}

bool buttonReleased() {
#ifdef STM32F407VGT6
    return !(gpio_port_read(GPIOA) & GPIO0);
#endif
#ifdef STM32F405RGT6
    return !(gpio_port_read(GPIOC) & GPIO4);
#endif
#ifdef STM32F411RG
    return !(gpio_port_read(GPIOB) & GPIO1);
#endif
}
