/*
 * This file is a part of Iskra JS Arduino SDK.
 *
 * Product page: https://amperka.ru/product/iskra-js
 * © Amperka LLC (https://amperka.com, dev@amperka.com)
 * 
 * Author: Yury Botov <by@amperka.com>
 * License: GPLv3, all text here must be included in any redistribution.
 */

#ifndef __JUMPERS_H__
#define __JUMPERS_H__

// start address of application memory (1 sector of FLASH)
#define ROM_APP_START 0x08004000
// start address of application RAM
#define RAM_APP_START 0x20000800
// address of USB FS vector
#define USB_FS_VECTOR 0x0800014c

// jump to application start
void appJumper(void);

// jump to bootloader restart
void resetJumper(void);

// jump to "relax" loop - prevent return to user application before rewriting end
void relaxJumper(void);


void external_otg_fs_isr(void);

void otg_fs_isr_real_handler(void);
void otg_fs_isr_broker(void);
void sys_tick_real_handler(void);
void sys_tick_handler_broker(void);

#endif
