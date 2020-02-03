/*
 * This file is a part of Iskra JS Arduino SDK.
 *
 * Product page: https://amperka.ru/product/iskra-js
 * © Amperka LLC (https://amperka.com, dev@amperka.com)
 * 
 * Author: Yury Botov <by@amperka.com>
 * License: GPLv3, all text here must be included in any redistribution.
 */

#ifndef __SERIAL_BUF_H__
#define __SERIAL_BUF_H__

#include <stdbool.h>
#include <stdint.h>

#define SERIALBUFLEN 128

typedef struct {
    int16_t first;
    int16_t last;
    int16_t len;
    uint8_t buf[SERIALBUFLEN];
} bufSerial;

void initBufSerial(bufSerial* buf);

bool canWrite(bufSerial* buf);
bool canRead(bufSerial* buf);

int16_t getBufSerial(bufSerial* buf);
void putBufSerial(bufSerial* buf, uint8_t c);

int16_t length(bufSerial* buf);

#endif