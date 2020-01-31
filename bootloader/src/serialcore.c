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

// In interrupt solve if needs to send
void cdcacm_sync(void) {
    while( serialOut.len > 0) {
        cdcacm_data_tx_all(usbdDevice);
    }
}

// Send packet to USB
void cdcacm_data_tx_all(usbd_device* usbd_dev) {
    char buf[64];
    int len = (serialOut.len > 60) ? 60 : serialOut.len;
    for (int i = 0; i < len; i++)
        buf[i] = (char)getBufSerial(&serialOut);
    usbd_ep_write_packet(usbd_dev, 0x83, buf, len);
}

// RX and TX buffers initialisation
void cdcacm_init(void) {
    initBufSerial(&serialIn);
    initBufSerial(&serialOut);
}

// TX buffer is not full
bool cdcacm_out_ready(void) { 
    return canWrite(&serialOut);
}

// Put char to TX buffer
void cdcacm_putc(uint8_t c) {
    putBufSerial(&serialOut, c);
    if (serialOut.len > 60) {
        cdcacm_data_tx_all(usbdDevice);
    }
}

// RX buffer is not empty
bool cdcacm_in_ready(void) {
    return canRead(&serialIn);
}

// read char from RX buffer
int cdcacm_getc(void) {
    return getBufSerial(&serialIn);
}
