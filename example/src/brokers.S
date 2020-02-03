/* 

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
    //cpsid i
    //bl application_sys_tick_handler + 1
    movw r6, #0x0800 ;  // 0x800 - 8 (SYSTICK_HANDLER)
    movt r6, #0x2000
    ldr pc,[r6,#-8]
.endfunc


// redirect to bootloader USB FS handler
.global otg_fs_isr
.func otg_fs_isr
.type otg_fs_isr, STT_FUNC
otg_fs_isr:
    //cpsid i
    movw r6, #0x0800 ;  // 0x800 - 4 (USB_FS_HANDLER)
    movt r6, #0x2000
    ldr pc,[r6,#-4]
.endfunc
