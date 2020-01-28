/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>
 * Copyright (C) 2011 Damjan Marion <damjan.marion@gmail.com>
 * Copyright (C) 2011 Mark Panajotovic <marko@electrontube.org>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "iskrajs.h"

#include <libopencm3/cm3/cortex.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#define DELAY 30000000

int main(void)
{	
	SCB_VTOR = 0x08004000;
//	nvic_enable_irq(NVIC_TIM5_IRQ);
	cm_enable_interrupts();
#ifdef STM32F407VGT6
	rcc_periph_clock_enable(RCC_GPIOD);
	gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT,
			GPIO_PUPD_NONE, GPIO12 | GPIO13 | GPIO14 | GPIO15);
	gpio_set(GPIOD, GPIO12 | GPIO14);
	while(1) {
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
	while(1) {
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
	while(1) {
		gpio_toggle(GPIOB, GPIO2 | GPIO12);
		for (int i = 0; i < DELAY; i++) {
			__asm__("nop");
		}
	}
#endif

	return 0;
}
