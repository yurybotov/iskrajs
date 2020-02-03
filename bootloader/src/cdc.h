#ifndef __CDC_H__
#define __CDC_H__

void cdcacm_set_config(usbd_device* usbd_dev, uint16_t wValue);

void cdcacm_data_rx_cb(usbd_device* usbd_dev, uint8_t ep);

#endif
