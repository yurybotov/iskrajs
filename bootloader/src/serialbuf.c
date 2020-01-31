/*
 * This file is a part of Iskra JS Arduino SDK.
 *
 * Product page: https://amperka.ru/product/iskra-js
 * © Amperka LLC (https://amperka.com, dev@amperka.com)
 * 
 * Author: Yury Botov <by@amperka.com>
 * License: GPLv3, all text here must be included in any redistribution.
 */

#include "serialbuf.h"

// initialisation
void initBufSerial(bufSerial* buf) {
    buf->len = buf->first = buf->last = 0;
}

// true if buffer is not full
bool canWrite(bufSerial* buf) { return (buf->len < SERIALBUFLEN); }

// true i buffer is not empty
bool canRead(bufSerial* buf) { return (buf->len > 0); }

// get character from buffer. -1 if buffer empty.
int16_t getBufSerial(bufSerial* buf) {
    int16_t c = -1;
    if (canRead(buf)) {
        c = buf->buf[buf->last];
        buf->last++;
        if (buf->last == SERIALBUFLEN)
            buf->last = 0;
        buf->len--;
    }
    return c;
}

// push character to buffer
void putBufSerial(bufSerial* buf, uint8_t c) {
    if (canWrite(buf)) {
        buf->buf[buf->first] = c;
        buf->first++;
        if (buf->first == SERIALBUFLEN)
            buf->first = 0;
        buf->len++;
    }
}

// get utilisation size
int16_t length(bufSerial* buf) {
    return buf->len;
}
