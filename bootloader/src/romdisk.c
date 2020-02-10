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

#include "jumpers.h"
#include "romdisk.h"

extern volatile bool otherBlock;

#define WBVAL(x) ((x)&0xFF), (((x) >> 8) & 0xFF)
#define QBVAL(x) \
    ((x)&0xFF), (((x) >> 8) & 0xFF), (((x) >> 16) & 0xFF), (((x) >> 24) & 0xFF)

#define SECTOR_SIZE 512
#define SECTOR_COUNT (2048 - 32)
#define START_DATA_SECTOR 69

// sector 45
const uint8_t Iskrajs_h[] =
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
    "\n";

// sector 49
const uint8_t Readme[] =
    "Readme\n\n"
    "This is IskraJS bootloader MSD device.\n"
    "Just copy you IskraJS firmware in .bin format to this disk.\n\n"
    "You firmware must be compiled for ROM start 0x08004000 and RAM start 0x20000800.\n"
    "If somthing is wrong: copy/replace `iskrajs.h` from here to you Arduino directory.\n"
    "\n";

// sector 00
static const uint8_t BootSector[] = {
    0xeb, 0x3c, 0x90, // code to jump to the bootstrap code
    'm', 'k', 'f', 's', '.', 'f', 'a', 't', 0x0, // OEM ID
    0x2, 0x4, // bytes per sector
    0x1, // sectors per cluster
    0x0, // # of reserved sectors (1 boot sector)
    0x2, // FAT copies (2)
    0x0, 0x2, // root entries (512)
    0x0, 0x8, // total number of sectors
    0xf8, // media descriptor (0xF8 = Fixed disk)
    0x2, 0x0, // sectors per FAT (2)
    0x3e, 0x0, // sectors per track ()
    0x20, 0x0, // number of heads ()
    0x0, 0x8, 0x0, 0x0, // hidden sectors ()
    0x0, 0x0, 0x0, 0x0, // large number of sectors () 
    0x80, // drive number ()
    0x1, // reserved
    0x29, // extended boot signature
    0x48, 0x56, 0x38, 0xf3, // volume serial number
    'I', 'S', 'K', 'R', 'A', 'J', 'S', ' ', ' ', ' ', ' ', // volume label
    'F', 'A', 'T', '1', '2', ' ', ' ', ' ', // filesystem type
    0xe, 0x1f, 0xbe, 0x5b, 0x7c, 0xac, 0x22, 0xc0, 0x74, 0xb, // print message and stop
    0x56, 0xb4, 0xe, 0xbb, 0x7, 0x0, 0xcd, 0x10, 0x5e, 0xeb, 
    0xf0, 0x32, 0xe4, 0xcd, 0x16, 0xcd, 0x19, 0xeb, 0xfe, 
    'T','h','i','s',' ','i','s',' ','n','o','t',' ','a',' ',
    'b','o','o','t','a','b','l','e',' ','d','i','s','k','.',
    ' ',' ','P','l','e','a','s','e',' ','i','n','s','e','r',
    't',' ','a',' ','b','o','o','t','a','b','l','e',' ','f',
    'l','o','p','p','y',' ','a','n','d', 0xd, 0xa, 'p','r',
    'e','s','s',' ','a','n','y',' ','k','e','y',' ','t','o',
    ' ','t','r','y',' ','a','g','a','i','n',' ', 0x2e, 0x2e,
    0x2e, 0x20, 0xd, 0xa
};

// sectors 01 03
const uint8_t Fat[] = {
    0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00      
};

// sectors 05
const uint8_t Root[] = {
    'I','S','K','R','A','J','S',' ',' ',' ',' ',
    0x8, 0x0, 0x0, 0x91, 0xbd, 0x49, 0x50, 0x49, 0x50, 0x0, 0x0,
    0xb2, 0x6a, // time
    0x4a, 0x50, // date
    0x0, 0x0, // start cluster
    0x0, 0x0, 0x0, 0x0, // file size

    0x42, // VFAT
    ' ', 0,'I',0,'n',0,'f',0,'o',0, 0xf, 0,0x72,'r',0,'m',0,
    'a',0,'t',0,'i',0,'o',0,0,0,'n', 0x0, 0x0, 0x0, 0x1,
    'S',0,'y',0,'s',0,'t',0,'e',0,0xf,0, 0x72,'m',0,' ',0,
    'V',0,'o',0,'l',0,'u',0,0,0,'m',0,'e',0, 
    'S','Y','S','T','E','M','~','1',' ',' ',' ',
    0x16, 0x0, 0x24, 0xd4, 0xa5, 0x49, 0x50, 0x49, 0x50, 0x0, 0x0, 
    0xd5, 0xa5, // time
    0x49, 0x50, // date
    0x2, 0x0, // start cluster
    0x0, 0x0, 0x0, 0x0, // file size
    
    'I','S','K','R','A','J','S',' ','H',' ',' ',
    0x20, 0x18, 0x61, 0xfb, 0xa5, 0x49, 0x50, 0x49, 0x50, 0x0, 0x0,
    0xc2, 0x7b, // time
    0x33, 0x50, // date
    0x4, 0x0, // start cluster
    QBVAL(sizeof(Iskrajs_h) - 1),//0xb3, 0x1, 0x0, 0x0, // file size
    
    'R','E','A','D','M','E',' ',' ','T','X','T', 
    0x20, 0x18, 0x88, 0xfb, 0xa5, 0x49, 0x50, 0x4a, 0x50, 0x0, 0x0,
    0x6c, 0x4d, // time
    0x33, 0x50, // date
    0x5, 0x0, // start cluster
    QBVAL(sizeof(Readme) - 1),//0x11, 0x1, 0x0, 0x0, // file size
    
    0x41, // VFAT
    '.',0,'f',0,'s',0,'e',0,'v',0, 0xf,0, 0xda,'e',0,'n',0,'t',0,'s',0,'d', 
    0x0, 0x0, 0x0, 0x0, 0x0, 0xff, 0xff, 0xff, 0xff, 
    'F','S','E','V','E','N','~','1',' ',' ',' ',
    0x12, 0x0, 0xb5, 0xb1, 0x6a, 0x4a, 0x50, 0x4a, 0x50, 0x0, 0x0,
    0xb1, 0x6a, // time
    0x4a, 0x50, // date
    0x6, 0x0, // start cluster
    0x0, 0x0, 0x0, 0x0 // file size
};

// sector 37
const uint8_t DirMs[] = {
    0x2e, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
    0x10, // attribute
    0x0, 0x24, 0xd4, 0xa5, 0x49, 0x50, 0x49, 0x50, 0x0, 0x0,
    0xd5, 0xa5, // time
    0x49, 0x50, // date
    0x2, 0x0, // start sector 
    0x0, 0x0, 0x0, 0x0, // file size

    0x2e, '.', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
    0x10, // attribute
    0x0, 0x24, 0xd4, 0xa5, 0x49, 0x50, 0x49, 0x50, 0x0, 0x0,
    0xd5, 0xa5, // time
    0x49, 0x50, // date
    0x0, 0x0, // start sector
    0x0, 0x0, 0x0, 0x0, // file size

    0x42, 'G',0,'u',0,'i',0,'d',0,0,0,
    0xf, // attribute
    0x0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0x0, 0x0, 0xff, 0xff, 0xff, 0xff,
    0x1, 'I',0,'n',0,'d',0,'e',0,'x',0,0xf,0,0xff,'e',0,'r',0,
    'V',0,'o',0,'l',0,'u',0,0,0,'m',0,'e',0,
    'I','N','D','E','X','E','~','1',' ',' ',' ',' ', 0x0,
    0x31, 0xd4, 0xa5, 
    0x49, 0x50, 
    0x49, 0x50,
    0x0, 0x0, 
    0xd5, 0xa5, // time
    0x49, 0x50, // date
    0x3, 0x0, // start sector
    0x4c, 0x0, 0x0, 0x0 // file size
};

// sector 53
const uint8_t DirApple[] = {
    0x2e,' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
    0x32, // attribute
    0x0, 0xb5, 0xb1, 0x6a, 0x4a, 0x50, 0x4a, 0x50, 0x0, 0x0, 
    0xee, 0x6a, // time
    0x4a, 0x50, // date
    0x6, 0x0, // start sector
    0x0, 0x0, 0x0, 0x0, // file size

    0x2e,'.',' ',' ',' ',' ',' ',' ',' ',' ',' ',
    0x10, // attribute
    0x0, 0xb5, 0xb1, 0x6a, 0x4a, 0x50, 0x4a, 0x50, 0x0, 0x0,
    0xb1, 0x6a, // time
    0x4a, 0x50, // date
    0x0, 0x0, // start sector
    0x0, 0x0, 0x0, 0x0, // file size

    0x42, 
    'd', 0x0, 0x0, 0x0, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xf, 0x0, 0xda, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0x0, 0x0, 0xff, 0xff, 0xff, 0xff,
    0x1,'f',0,'s',0,'e',0,'v',0,'e',0,0xf,0,0xda,'n',0,
    't',0,'s',0,'d',0,'-',0,'u',0,0,0,'u',0,'i',0,
    'F','S','E','V','E','N','~','1',' ',' ',' ',
    0x20, // attribute
    0x0, 0x5, 0xb2, 0x6a, 0x4a, 0x50, 0x4a, 0x50, 0x0, 0x0,
    0xee, 0x6a, // time
    0x4a, 0x50, // date
    0x7, 0x0, // start sector
    0x24, 0x0, 0x0, 0x0, // file size

    0x42, 
    '3',0,'b',0,'a',0,0,0, 0xff, 0xff, 0xf, 0x0, 0x34, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0x0, 0x0, 0xff, 0xff, 0xff, 0xff,
    0x1,'f',0,'c',0,'0',0,'0',0,'7',0, 0xf,0,'4','4',0,
    'f',0,'d',0,'d',0,'5',0,'c',0,0,0,'6',0,'f',0,
    'F','C','0','0','7','4','~','1',' ',' ',' ',
    0x20, // attribute
    0x0, 0x17, 0xee, 0x6a, 0x4a, 0x50, 0x4a, 0x50, 0x0, 0x0,
    0xee, 0x6a, // time
    0x4a, 0x50, // date
    0x8, 0x0, // start sector
    0x34, 0x0, 0x0, 0x0, // file size

    0x42,
    '3',0,'b',0,'b',0,0,0, 0xff, 0xff, 0xf, 0x0, 0x94, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0x0, 0x0, 0xff, 0xff, 0xff, 0xff,
    0x1,'f',0,'c',0,'0',0,'0',0,'7',0,0xf,0,0x94,'4',0,
    'f',0,'d',0,'d',0,'5',0,'c',0,0,0,'6',0,'f',0,
    'F','C','0','0','7','4','~','2',' ',' ',' ',
    0x20, // attribute
    0x0, 0x18, 0xee, 0x6a, 0x4a, 0x50, 0x4a, 0x50, 0x0, 0x0, 
    0xee, 0x6a, // time
    0x4a, 0x50, // date
    0x9, 0x0, // start sector
    0x4c, 0x0, 0x0, 0x0 // file size
};

// sector 41
const uint8_t GuidMs[] = {
    '{', 0, '4', 0, '7', 0, '3', 0, '8', 0, '2', 0, '6', 0, 'D', 0,
    '5', 0, '-', 0, '6', 0, 'B', 0, '3', 0, '2', 0, '-', 0, '4', 0,
    '0', 0, '5', 0, 'D', 0, '-', 0, '9', 0, 'D', 0, '2', 0, 'E', 0,
    '-', 0, '2', 0, '5', 0, 'C', 0, 'A', 0, '5', 0, '4', 0, 'A', 0,
    '9', 0, 'D', 0, '5', 0, '4', 0, 'F', 0, '}', 0
};

// sector 57
const uint8_t GuidApple[] = "7F739438-73B9-45BF-9C9C-00AEED8E56DD";

// sector 61
const uint8_t AppFile1[] = {
    0x1f, 0x8b, 0x8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x13, 0x32, 0xa, 
    0xf6, 0x71, 0x51, 0x79, 0xb0, 0xba, 0x50, 0x91, 0x1, 0x8, 0x76,
    0x7e, 0x3e, 0x76, 0xf5, 0x6f, 0x9, 0xc3, 0x1f, 0x20, 0x93, 0x89,
    0x1, 0xa, 0x0, 0x0, 0x0, 0x0, 0xff, 0xff, 0x3, 0x0, 0x91, 0x4d,
    0x82, 0x73, 0x21
};

// sector 65
const uint8_t AppFile2[] = {
    0x1f, 0x8b, 0x8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x13, 0x32, 0xc,
    0xf6, 0x71, 0x99, 0x78, 0xfa, 0xa, 0xbb, 0x15, 0x3, 0x3, 0x83,
    0x5e, 0x5a, 0x71, 0x6a, 0x59, 0x6a, 0x5e, 0x49, 0x71, 0x8a, 0x7e,
    0x71, 0x8e, 0x6e, 0x72, 0x7e, 0x6e, 0x41, 0x62, 0x9, 0x43, 0xdd,
    0xaf, 0x63, 0x57, 0xff, 0x96, 0x30, 0xfc, 0x1, 0xca, 0x33, 0x32,
    0xd4, 0x23, 0x38, 0x8a, 0x0, 0x0, 0x0, 0x0, 0xff, 0xff, 0x3, 0x0,
    0xee, 0x17, 0x2f, 0x1c, 0x3a
};

// work area: 69?

static uint8_t* romdata = (uint8_t*)ROM_APP_START; // 1 sector of flash, in 0 sector we has bootloader

int romdisk_init(void) {
    return 0;
}

int romdisk_read(uint32_t lba, uint8_t* copy_to) {
    memset(copy_to, 0, SECTOR_SIZE);
    switch (lba) {
    case 0: // sector 0 is the boot sector
        memcpy(copy_to, BootSector, sizeof(BootSector));
        copy_to[SECTOR_SIZE - 2] = 0x55;
        copy_to[SECTOR_SIZE - 1] = 0xAA;
        break;
    case 1: // FAT 1st copy
    case 3: // FAT 2nd copy
        memcpy(copy_to, Fat, sizeof(Fat));
        break;
    case 5: // ROOT dir
        memcpy(copy_to, Root, sizeof(Root));
        break;
    case 37: // System Volume Information dir
        memcpy(copy_to, DirMs, sizeof(DirMs));
        break;
    case 41: // Windows guid file
        memcpy(copy_to, GuidMs, sizeof(GuidMs));
        break;
    case 45: // h-file
        memcpy(copy_to, Iskrajs_h, sizeof(Iskrajs_h));
        break;
    case 49: // readme
        memcpy(copy_to, Readme, sizeof(Readme));
        break;
    case 53: // apple dir
        memcpy(copy_to, DirApple, sizeof(DirApple));
        break;
    case 57: // apple guid
        memcpy(copy_to, GuidApple, sizeof(GuidApple));
        break;
    case 61: // apple hidden file 1
        memcpy(copy_to, AppFile1, sizeof(AppFile1));
        break;
    case 65: // apple hidden file 2
        memcpy(copy_to, AppFile2, sizeof(AppFile2));
        break;
    default:
        break;
    }
    return 0;
}

static uint8_t shift = 0;
static bool firmwareIsRight = false;

int romdisk_write(uint32_t lba, const uint8_t* copy_from) {
    // testing firmware structure
    if (lba == START_DATA_SECTOR) {
        if (((uint32_t*)(copy_from))[0] ==
#ifdef STM32F407VGT6
            0x20020000
#endif
#ifdef STM32F405RGT6
            0x20020000
#endif
#ifdef STM32F411RE
            0x20020000
#endif
        ) { // different stack start for different controllers
            firmwareIsRight = true;
        } else {
            firmwareIsRight = false;
        }
        relaxJumper();
    }
    if (lba >= START_DATA_SECTOR) {
        if (firmwareIsRight) { // if firmware structure is ok

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
#ifndef STM32F411RE
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
