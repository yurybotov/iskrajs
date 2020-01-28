#ifndef __USB__H__
#define __USB__H__

#include <libopencm3/usb/cdc.h>
#include <libopencm3/usb/msc.h>
#include <libopencm3/usb/usbd.h>

void cdcacm_init(void);
void cdcacm_set_config(usbd_device* usbd_dev, uint16_t wValue);

#endif