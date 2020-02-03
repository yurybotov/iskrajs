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

// do local and redirect to bootloader systick address
.extern application_sys_tick_handler
.global sys_tick_handler
.func sys_tick_handler
.type sys_tick_handler, STT_FUNC

sys_tick_handler:
    push {r0-r6, lr}
    bl application_sys_tick_handler
    pop {r0-r6, lr}
    movw r3, #0x0800 ;  // 0x800 - 8 (SYSTICK_HANDLER)
    movt r3, #0x2000
    ldr pc,[r3,#-8]
.endfunc

// redirect to bootloader USB FS handler
.global otg_fs_isr
.func otg_fs_isr
.type otg_fs_isr, STT_FUNC

otg_fs_isr:
    movw r3, #0x0800 ;  // 0x800 - 4 (USB_FS_HANDLER)
    movt r3, #0x2000
    ldr pc,[r3,#-4]
.endfunc

.ascii "Amperka IskraJS bootloader v.1.0.0"
