/*
 * This file is a part of Iskra JS Arduino SDK.
 *
 * Product page: https://amperka.ru/product/iskra-js
 * © Amperka LLC (https://amperka.com, dev@amperka.com)
 * 
 * Author: Yury Botov <by@amperka.com>
 * License: GPLv3, all text here must be included in any redistribution.
 */

.syntax unified
.thumb
.section .text

.extern otg_fs_isr_real_handler
.extern sys_tick_real_handler

// otg fs interrupt broker
.global otg_fs_isr_broker
.func otg_fs_isr_broker
.type otg_fs_isr_broker, STT_FUNC
otg_fs_isr_broker:
	push {r0-r6, lr}
	bl otg_fs_isr_real_handler
	pop {r0-r6, lr}
	bx lr
.endfunc

// systick interrupt broker
.global sys_tick_handler_broker
.func sys_tick_handler_broker
.type sys_tick_handler_broker, STT_FUNC
sys_tick_handler_broker:
	push {r0-r6, lr}
	bl sys_tick_real_handler
	pop {r0-r6, lr}
	bx lr
.endfunc
