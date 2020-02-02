/*
 * This file is a part of Iskra JS Arduino SDK.
 *
 * Product page: https://amperka.ru/product/iskra-js
 * © Amperka LLC (https://amperka.com, dev@amperka.com)
 * 
 * Author: Yury Botov <by@amperka.com>
 * License: GPLv3, all text here must be included in any redistribution.
 */

#include <libopencm3/cm3/cortex.h>
#include <libopencm3/stm32/flash.h>
#include <string.h>

#include "romdisk.h"
#include "jumpers.h"

extern volatile bool otherBlock;

#define WBVAL(x) ((x)&0xFF), (((x) >> 8) & 0xFF)
#define QBVAL(x) \
    ((x)&0xFF), (((x) >> 8) & 0xFF), (((x) >> 16) & 0xFF), (((x) >> 24) & 0xFF)

#define SECTOR_SIZE 512
#define SECTOR_COUNT (2048 - 32)
#define START_DATA_SECTOR 56

static const uint8_t BootSector[] = {   // sector 0 cluster 0
    0xeb, 0x3c, 0x90,                                       // code to jump to the bootstrap code
    'M','S','D','O','S','5','.','0', 0x00,                  // OEM ID
    0x2, 0x4,                                               // bytes per sector
    0x4,                                                    // sectors per cluster 
    0x0,                                                    // # of reserved sectors (1 boot sector) 
    0x2,                                                    // FAT copies (2)
    0x0, 0x2,                                               // root entries (512)
    0x0, 0x8,                                               // total number of sectors
    0xf8,                                                   // media descriptor (0xF8 = Fixed disk) 
    0x2, 0x0,                                               // sectors per FAT (2)
    0x3f, 0x0,                                              // sectors per track ()
    0xff, 0x0,                                              // number of heads ()
    0x0, 0x8, 0x0, 0x0,                                     // hidden sectors ()  
    0x0, 0x0, 0x0, 0x0,                                     // large number of sectors ()
    0x80,                                                   // drive number ()
    0x1,                                                    // reserved
    0x29,                                                   // extended boot signature
    0xb, 0x8d, 0x9c, 0x0,                                   // volume serial number
    'I','S','K','R','A','J','S',' ',' ',' ',' ',            // volume label
    'F','A','T','1','2',' ',' ',' ',                        // filesystem type
    
    // DOS BOOT RECORD

    0x33, 0xc9, 0x8e, 0xd1, 0xbc, 0xf0, 0x7b, 0x8e, 0xd9, 0xb8, 0x0, 0x20,
    0x8e, 0xc0, 0xfc, 0xbd, 0x0, 0x7c, 0x38, 0x4e, 0x24, 0x7d, 0x24, 0x8b, 
    0xc1, 0x99, 0xe8, 0x3c, 0x1, 0x72, 0x1c, 0x83, 0xeb, 0x3a, 0x66, 0xa1,
    0x1c, 0x7c, 0x26, 0x66, 0x3b, 0x7, 0x26, 0x8a, 0x57, 0xfc, 0x75, 0x6,
    0x80, 0xca, 0x2, 0x88, 0x56, 0x2, 0x80, 0xc3, 0x10, 0x73, 0xeb, 0x33,
    0xc9, 0x8a, 0x46, 0x10, 0x98, 0xf7, 0x66, 0x16, 0x3, 0x46, 0x1c, 0x13, 
    0x56, 0x1e, 0x3, 0x46, 0xe, 0x13, 0xd1, 0x8b, 0x76, 0x11, 0x60, 0x89,
    0x46, 0xfc, 0x89, 0x56, 0xfe, 0xb8, 0x20, 0x0, 0xf7, 0xe6, 0x8b, 0x5e,
    0xb, 0x3, 0xc3, 0x48, 0xf7, 0xf3, 0x1, 0x46, 0xfc, 0x11, 0x4e, 0xfe,
    0x61, 0xbf, 0x0, 0x0, 0xe8, 0xe6, 0x0, 0x72, 0x39, 0x26, 0x38, 0x2d,
    0x74, 0x17, 0x60, 0xb1, 0xb, 0xbe, 0xa1, 0x7d, 0xf3, 0xa6, 0x61, 0x74,
    0x32, 0x4e, 0x74, 0x9, 0x83, 0xc7, 0x20, 0x3b, 0xfb, 0x72, 0xe6, 0xeb,
    0xdc, 0xa0, 0xfb, 0x7d, 0xb4, 0x7d, 0x8b, 0xf0, 0xac, 0x98, 0x40, 0x74,
    0xc, 0x48, 0x74, 0x13, 0xb4, 0xe, 0xbb, 0x7, 0x0, 0xcd, 0x10, 0xeb, 0xef,
    0xa0, 0xfd, 0x7d, 0xeb, 0xe6, 0xa0, 0xfc, 0x7d, 0xeb, 0xe1, 0xcd, 0x16,
    0xcd, 0x19, 0x26, 0x8b, 0x55, 0x1a, 0x52, 0xb0, 0x1, 0xbb, 0x0, 0x0, 0xe8,
    0x3b, 0x0, 0x72, 0xe8, 0x5b, 0x8a, 0x56, 0x24, 0xbe, 0xb, 0x7c, 0x8b, 0xfc,
    0xc7, 0x46, 0xf0, 0x3d, 0x7d, 0xc7, 0x46, 0xf4, 0x29, 0x7d, 0x8c, 0xd9,
    0x89, 0x4e, 0xf2, 0x89, 0x4e, 0xf6, 0xc6, 0x6, 0x96, 0x7d, 0xcb, 0xea, 0x3,
    0x0, 0x0, 0x20, 0xf, 0xb6, 0xc8, 0x66, 0x8b, 0x46, 0xf8, 0x66, 0x3, 0x46,
    0x1c, 0x66, 0x8b, 0xd0, 0x66, 0xc1, 0xea, 0x10, 0xeb, 0x5e, 0xf, 0xb6,
    0xc8, 0x4a, 0x4a, 0x8a, 0x46, 0xd, 0x32, 0xe4, 0xf7, 0xe2, 0x3, 0x46, 0xfc,
    0x13, 0x56, 0xfe, 0xeb, 0x4a, 0x52, 0x50, 0x6, 0x53, 0x6a, 0x1, 0x6a,  
    0x10, 0x91, 0x8b, 0x46, 0x18, 0x96, 0x92, 0x33, 0xd2, 0xf7, 0xf6, 0x91,
    0xf7, 0xf6, 0x42, 0x87, 0xca, 0xf7, 0x76, 0x1a, 0x8a, 0xf2, 0x8a, 0xe8,  
    0xc0, 0xcc, 0x2, 0xa, 0xcc, 0xb8, 0x1, 0x2, 0x80, 0x7e, 0x2, 0xe, 0x75,
    0x4, 0xb4, 0x42, 0x8b, 0xf4, 0x8a, 0x56, 0x24, 0xcd, 0x13, 0x61, 0x61, 0x72,
    0xb, 0x40, 0x75, 0x1, 0x42, 0x3, 0x5e, 0xb, 0x49, 0x75, 0x6, 0xf8, 0xc3,
    0x41, 0xbb, 0x0, 0x0, 0x60, 0x66, 0x6a, 0x0, 0xeb, 0xb0, 
    'B','O','O','T','M','G','R',' ',' ',' ',' ',
    0xd, 0xa,
    'R','e','m','o','v','e',' ','d','i','s','k','s',' ',
    'o','r',' ','o','t','h','e','r',' ','m','e','d','i','a',
    0x2e, 0xff, 0xd, 0xa, 
    'D','i','s','k',' ','e','r','r','o','r',
    0xff, 0xd, 0xa,
    'P','r','e','s','s',' ','a','n','y',' ','k','e','y',' ',
    't','o',' ','r','e','s','t','a','r','t',
    0xd, 0xa, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xac, 0xcb, 0xd8, 0x55, 0xaa
};

const uint8_t Fat[] = { // cluster 1 sector 4 and sector 6
    0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

// readme file
const uint8_t Readme[] = // cluster 12 sector 48
    "Readme\n\n"
    "This is IskraJS bootloader MSD device.\n"
    "Just copy you IskraJS firmware in .bin format to this disk.\n\n"
    "You firmware must be compiled for ROM start 0x08004000 and RAM start 0x20000800.\n"
    "If somthing is wrong: copy/replace `iskrajs.h` from here to you Arduino directory.\n"
    "\n"
;

const uint8_t Iskrajs_h[] = { // cluster 13 sector 52
    "/*\n"
    "* This file is a part of Iskra JS Arduino SDK.\n"
    "*\n"
    "* Product page: https://amperka.ru/product/iskra-js\n"
    "* © Amperka LLC (https://amperka.com, dev@amperka.com)\n"
    "*\n"
    "* Author: Yury Botov <by@amperka.com>\n"
    "* License: GPLv3, all text here must be included in any redistribution.\n"
    "*/\n"
    "\n"
    "#ifndef __ISKRAJS_H__\n"
    "#define __ISKRAJS_H__\n"
    "\n"
    "#define BOOTLOADER_VERSION 10000\n"
    "#define APPLICATION_START 0x08004000\n"
    "#define APPLICATION_RAM 0x20000800\n"
    "\n"
    "#endif\n"
    "\n"
};

const uint8_t Root[] = { // cluster 2 sector 8
    // VOLUME
    'I','S','K','R','A','J','S',' ',' ',' ',' ',
    0x8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x4e, 0x4d, // date  
    0x33, 0x50, // time 
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0,

    // System Volume Information
    0x42,' ',0,'I',0,'n',0,'f', 0,'o',0,
    0xf, 0x0, 0x72, 
    'r',0,'m',0,'a',0,'t',0,'i',0,'o',0,
    0x0, 0x0, 'n', 0x0, 0x0, 0x0, 0x1,
    'S',0,'y',0,'s',0,'t',0,'e', 
    0x0, 0xf, 0x0, 0x72, 
    'm',0,' ',0,'V',0,'o',0,'l',0,'u',0,0,0,'m',0,'e',0,
    'S','Y','S','T','E','M','~','1',' ',' ',' ',  
    0x16, // attribute
    0x0, 0xc1, 0x4e, 0x4d, 0x33, 0x50, 0x33, 0x50, 0x0, 0x0, 
    0x4f, 0x4d, // time  
    0x33, 0x50, // date
    0x2, 0x0, // start cluster 
    0x0, 0x0, 0x0, 0x0, // file size

    // readme.txt
    'R','E','A','D','M','E',' ',' ','T','X','T', 
    0x20, // attribute
    0x18, 0x9b, 0x6b, 0x4d, 0x33, 0x50, 0x33, 0x50, 0x0, 0x0, 
    0x6c, 0x4d, // time  
    0x33, 0x50, // date
    0x4, 0x0, // start cluster
    /*0x8*/ QBVAL(sizeof(Readme)-1),/*, 0x0, 0x0, 0x0*/ // file size

    // include file
    0x41,'i',0,'s',0,'k',0,'r',0,'a', 0x0, 0xf, 0x0, 0x75,
    'j',0,'s',0, 0x2e,0,'h', 0x0, 0x0, 0x0,  
    0xff, 0xff, 0x0, 0x0, 0xff, 0xff, 0xff, 0xff, 
    'I','S','K','R','A','J','S',' ','H',' ',' ',
    0x20, // attribute
    0x0, 0x4d, 0xc2, 0x7b, 0x33, 0x50, 0x33, 0x50, 0x0, 0x0, 
    0xc2, 0x7b, // time  
    0x33, 0x50, // date
    0x5, 0x0, // start cluster 
    QBVAL(sizeof(Iskrajs_h)-1), /*0xe5, 0x0, 0x0, 0x0*/ // file size
};

const uint8_t Dir[] = { // cluster 10 sector 40
    0x2e, ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
    0x10, // attribute
    0x0, 0xc1, 0x4e, 0x4d, 0x33, 0x50, 0x33, 0x50, 0x0, 0x0,
    0x4f, 0x4d, // time
    0x33, 0x50, // date
    0x2, 0x0, // start sector
    0x0, 0x0, 0x0, 0x0, // file size
    0x2e, ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
    0x10, // attribute
    0x0, 0xc1, 0x4e, 0x4d, 0x33, 0x50, 0x33, 0x50, 0x0, 0x0,
    0x4f, 0x4d, // time
    0x33, 0x50, // date,
    0x0, 0x0, // start sector
    0x0, 0x0, 0x0, 0x0, // file size
    0x42,'G',0,'u',0,'i',0,'d',  
    0x0, 0x0, 0x0, 0xf, 0x0, 0xff, 0xff, 0xff,  
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,  
    0xff, 0xff, 0x0, 0x0, 0xff, 0xff, 0xff, 0xff,  
    0x1,'I',0,'n',0,'d',0,'e',0,'x', 0x0, 0xf, 0x0, 0xff,
    'e',0,'r',0,'V',0,'o',0,'l',0,'u',0,0,0,'m',0,'e',0,
    'I','N','D','E','X','E','~','1',' ',' ',' ',
    0x20, // attribute
    0x0, 0xc4, 0x4e, 0x4d, 0x33, 0x50, 0x33, 0x50, 0x0, 0x0,
    0x4f, 0x4d, // time
    0x33, 0x50, // date
    0x3, 0x0, // start cluster
    0x4c, 0x0, 0x0, 0x0 // file size
};

const uint8_t Guid[] = { // cluster 11 sector 44
    '{', 0, '4', 0, '7', 0, '3', 0, '8', 0, '2', 0, '6', 0, 'D', 0,  
    '5', 0, '-', 0, '6', 0, 'B', 0, '3', 0, '2', 0, '-', 0, '4', 0,  
    '0', 0, '5', 0, 'D', 0, '-', 0, '9', 0, 'D', 0, '2', 0, 'E', 0,  
    '-', 0, '2', 0, '5', 0, 'C', 0, 'A', 0, '5', 0, '4', 0, 'A', 0,  
    '9', 0, 'D', 0, '5', 0, '4', 0, 'F', 0, '}', 0
};

// work area: cluster 14 sector 56

static uint8_t *romdata = (uint8_t *)ROM_APP_START; // 1 sector of flash, in 0 sector we has bootloader

int romdisk_init(void) {
    return 0;
}

int romdisk_read(uint32_t lba, uint8_t *copy_to) {
    memset(copy_to, 0, SECTOR_SIZE);
    switch (lba) {
    case 0: // sector 0 is the boot sector
        memcpy(copy_to, BootSector, sizeof(BootSector));
        break;
    case 4: // FAT 1st copy
    case 6: // FAT 2nd copy
        memcpy(copy_to, Fat, sizeof(Fat));
        break;
    case 8: // ROOT dir
        memcpy(copy_to, Root, sizeof(Root));
        break;
    case 40: // System Volume Information dir
        memcpy(copy_to, Dir, sizeof(Dir));
        break;
    case 44: // Windows guid file
        memcpy(copy_to, Guid, sizeof(Guid));
        break;
    case 48: // readme file
        memcpy(copy_to, Readme, sizeof(Readme));
        break;
    case 52: // include file for Arduino API
        memcpy(copy_to, Iskrajs_h, sizeof(Iskrajs_h));
        break;    
    default:
        break;
    }
    return 0;
}

static uint8_t shift = 0;
static bool firmwareIsRight = false;

int romdisk_write(uint32_t lba, const uint8_t *copy_from) {
    // testing firmware structure
    if(lba == START_DATA_SECTOR) {
        if(((uint32_t*)(copy_from))[0] == 0x20020000) { ////////////////////// todo сделать другие цифры для разных контроллеров
            firmwareIsRight = true;
        } else {
            firmwareIsRight = false;
        }
        relaxJumper();
    }
    if (lba >= START_DATA_SECTOR) {
        if(firmwareIsRight) { // if firmware structure is ok

            flash_unlock();

            switch (lba - START_DATA_SECTOR) {
            case 0: /*clear sector 1*/
                otherBlock = true;
                flash_erase_sector(1, 0);
                break;
            case 32: /*clear sector 2*/
                otherBlock = true;
                flash_erase_sector(2, 0);
                break;
            case 64: /*clear sector 3*/
                otherBlock = true;
                flash_erase_sector(3, 0);
                break;
            case 96: /*clear sector 4*/
                otherBlock = true;
                flash_erase_sector(4, 0);
                break;
            case 224: /*clear sector 5*/
                otherBlock = true;
                flash_erase_sector(5, 0);
                break;
            case 480: /*clear sector 6*/
                otherBlock = true;
                flash_erase_sector(6, 0);
                break;
            case 736: /*clear sector 7*/
                otherBlock = true;
                flash_erase_sector(7, 0);
                break;
#ifndef STM32F411RG 
            case 992: /*clear sector 8*/
                otherBlock = true;
                flash_erase_sector(8, 0);
                break;
            case 1248: /*clear sector 9*/
                otherBlock = true;
                flash_erase_sector(9, 0);
                break;
            case 1504: /*clear sector 10*/
                otherBlock = true;
                flash_erase_sector(10, 0);
                break;
            case 1760: /*clear sector 11*/
                otherBlock = true;
                flash_erase_sector(11, 0);
                break;
#endif // STM32F411RG 
            default:
                break;
            }

            if (lba >= START_DATA_SECTOR && lba < SECTOR_COUNT) {
                flash_program((uint32_t)romdata + (lba - START_DATA_SECTOR) * SECTOR_SIZE, copy_from, SECTOR_SIZE);
            }

            flash_lock();
        }
        // else do not write bad firmware
    }
    return 0;
}

int romdisk_blocks(void) { return SECTOR_COUNT; }
