#ifndef __SERIAL_BUF_H__
#define __SERIAL_BUF_H__

#include <stdbool.h>
#include "external.h"

#define SERIALBUFLEN 256

typedef signed short int16_t;
typedef unsigned char uint8_t;

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

#endif