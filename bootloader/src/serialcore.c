/*
 * This file is a part of Iskra JS Arduino SDK.
 *
 * Product page: https://amperka.ru/product/iskra-js
 * © Amperka LLC (https://amperka.com, dev@amperka.com)
 * 
 * Author: Yury Botov <by@amperka.com>
 * License: GPLv3, all text here must be included in any redistribution.
 */

#include "serialcore.h"
#include "serialbuf.h"

extern usbd_device* usbdDevice;

bufSerial serialIn;
bufSerial serialOut;

void cdcacm_data_tx_all(usbd_device* usbd_dev) {
    char buf[64];
    int len = (serialOut.len > 64) ? 64 : serialOut.len;
    for (int i = 0; i < len; i++)
        buf[i] = (char)getBufSerial(&serialOut);
    usbd_ep_write_packet(usbd_dev, 0x83, buf, len);
}

void cdcacm_init(void) {
    initBufSerial(&serialIn);
    initBufSerial(&serialOut);
}

bool cdcacm_out_ready(void) { return canWrite(&serialOut); }

void cdcacm_putc(uint8_t c) {
    putBufSerial(&serialOut, c);
    if (serialOut.len > 60) {
        cdcacm_data_tx_all(usbdDevice);
    }
}

bool cdcacm_in_ready(void) { return canRead(&serialIn); }

int cdcacm_getc(void) { return getBufSerial(&serialIn); }