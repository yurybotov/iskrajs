#ifndef __JUMPERS_H__
#define __JUMPERS_H__

// start address of application memory (1 sector of FLASH)
#define APP_START 0x08004000
// start address of application RAM
#define RAM_START 0x20001000
// address of USB FS vector
#define USB_FS_VECTOR 0x0800014c

// jump to application start
void appJumper(void);

// jump to bootloader restart
void resetJumper(void);

// jump to "relax" loop - prevent return to user application before rewriting end
void relaxJumper(void);

#endif
