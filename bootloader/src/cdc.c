/*
 * This file is a part of Iskra JS Arduino SDK.
 *
 * Product page: https://amperka.ru/product/iskra-js
 * © Amperka LLC (https://amperka.com, dev@amperka.com)
 * 
 * Author: Yury Botov <by@amperka.com>
 * License: GPLv3, all text here must be included in any redistribution.
 */

#include <stdlib.h>

#include <libopencm3/cm3/scb.h>

#include "usb.h"
#include "serialbuf.h"

extern usbd_device* usbdDevice;

/*
 * This notification endpoint isn't implemented. According to CDC spec it's
 * optional, but its absence causes a NULL pointer dereference in the
 * Linux cdc_acm driver.
 */
static const struct usb_endpoint_descriptor comm_endp[] = { { 
	.bLength = USB_DT_ENDPOINT_SIZE,
    .bDescriptorType = USB_DT_ENDPOINT,
    .bEndpointAddress = 0x84,
    .bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
    .wMaxPacketSize = 16,
    .bInterval = 255 
} };

static const struct usb_endpoint_descriptor data_endp[] = {
    { .bLength = USB_DT_ENDPOINT_SIZE,
        .bDescriptorType = USB_DT_ENDPOINT,
        .bEndpointAddress = 0x03,
        .bmAttributes = USB_ENDPOINT_ATTR_BULK,
        .wMaxPacketSize = 64,
        .bInterval = 1 },
    { .bLength = USB_DT_ENDPOINT_SIZE,
        .bDescriptorType = USB_DT_ENDPOINT,
        .bEndpointAddress = 0x83,
        .bmAttributes = USB_ENDPOINT_ATTR_BULK,
        .wMaxPacketSize = 64,
        .bInterval = 1 }
};

static const struct {
    struct usb_cdc_header_descriptor header;
    struct usb_cdc_call_management_descriptor call_mgmt;
    struct usb_cdc_acm_descriptor acm;
    struct usb_cdc_union_descriptor cdc_union;
} __attribute__((packed)) cdcacm_functional_descriptors = {
    .header = {
        .bFunctionLength = sizeof(struct usb_cdc_header_descriptor),
        .bDescriptorType = CS_INTERFACE,
        .bDescriptorSubtype = USB_CDC_TYPE_HEADER,
        .bcdCDC = 0x0110 },
    .call_mgmt = { 
		.bFunctionLength = sizeof(struct usb_cdc_call_management_descriptor), 
		.bDescriptorType = CS_INTERFACE, 
		.bDescriptorSubtype = USB_CDC_TYPE_CALL_MANAGEMENT, 
		.bmCapabilities = 0, .bDataInterface = 1 },
    .acm = { 
		.bFunctionLength = sizeof(struct usb_cdc_acm_descriptor), 
		.bDescriptorType = CS_INTERFACE, 
		.bDescriptorSubtype = USB_CDC_TYPE_ACM, 
		.bmCapabilities = 0 },
    .cdc_union = { 
		.bFunctionLength = sizeof(struct usb_cdc_union_descriptor), 
		.bDescriptorType = CS_INTERFACE, 
		.bDescriptorSubtype = USB_CDC_TYPE_UNION, 
		.bControlInterface = 2, 
		.bSubordinateInterface0 = 1 }
};

const struct usb_iface_assoc_descriptor iadcdc_descr[] = { { 
	.bLength = USB_DT_INTERFACE_ASSOCIATION_SIZE,
	.bDescriptorType = USB_DT_INTERFACE_ASSOCIATION,
	.bFirstInterface = 1,
	.bInterfaceCount = 2,
	.bFunctionClass = USB_CLASS_CDC,
	.bFunctionSubClass = USB_CDC_SUBCLASS_ACM,
	.bFunctionProtocol = USB_CDC_PROTOCOL_AT,
	.iFunction = 0 
} };

const struct usb_interface_descriptor comm_iface[] = { { 
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 1,
	.bAlternateSetting = 0,
	.bNumEndpoints = 1,
	.bInterfaceClass = USB_CLASS_CDC,
	.bInterfaceSubClass = USB_CDC_SUBCLASS_ACM,
	.bInterfaceProtocol = USB_CDC_PROTOCOL_AT,
	.iInterface = 0,

	.endpoint = comm_endp,

	.extra = &cdcacm_functional_descriptors,
	.extralen = sizeof(cdcacm_functional_descriptors) 
} };

const struct usb_interface_descriptor data_iface[] = { { 
	.bLength = USB_DT_INTERFACE_SIZE,
    .bDescriptorType = USB_DT_INTERFACE,
    .bInterfaceNumber = 2,
    .bAlternateSetting = 0,
    .bNumEndpoints = 2,
    .bInterfaceClass = USB_CLASS_DATA,
    .bInterfaceSubClass = 0,
    .bInterfaceProtocol = 0,
    .iInterface = 0,

    .endpoint = data_endp 
} };

static enum usbd_request_return_codes cdcacm_control_request(
    usbd_device* usbd_dev, struct usb_setup_data* req, uint8_t** buf,
    uint16_t* len,
    void (**complete)(usbd_device* usbd_dev, struct usb_setup_data* req)) {

    (void)complete;
    (void)buf;
    (void)usbd_dev;

    switch (req->bRequest) {
    case USB_CDC_REQ_SET_CONTROL_LINE_STATE: {
        return USBD_REQ_HANDLED;
    }
    case USB_CDC_REQ_SET_LINE_CODING:
        if (*len < sizeof(struct usb_cdc_line_coding)) {
            return USBD_REQ_NOTSUPP;
        }

        return USBD_REQ_HANDLED;
    }
    return USBD_REQ_NOTSUPP;
}

bufSerial serialIn;
bufSerial serialOut;

static void cdcacm_data_tx_all(usbd_device* usbd_dev) {
	char buf[64];
	int len = ( serialOut.len > 64)? 64 : serialOut.len;
	for (int i = 0; i < len; i++) buf[i] = (char)getBufSerial(&serialOut);
	usbd_ep_write_packet(usbd_dev, 0x83, buf, len);
}

void cdcacm_init(void) { initBufSerial(&serialIn); initBufSerial(&serialOut); }

bool cdcacm_out_ready(void) { return canWrite(&serialOut); }

void cdcacm_putc(uint8_t c) {
    putBufSerial(&serialOut, c);
    if(serialOut.len > 60) {
        cdcacm_data_tx_all( usbdDevice);
    }
}

bool cdcacm_in_ready(void) { return canRead(&serialIn); }

int cdcacm_getc(void) { return getBufSerial(&serialIn); }

static void cdcacm_data_rx_cb(usbd_device* usbd_dev, uint8_t ep) {
    (void)ep;

    char buf[64];
    int len = usbd_ep_read_packet(usbd_dev, 0x03, buf, 64);

    for (int i = 0; i < len; i++) {
        putBufSerial(&serialIn, buf[i] + 1);
    }

	for (int i = 0; i < len; i++) {
        cdcacm_putc( cdcacm_getc()+1);
    }
}

void cdcacm_set_config(usbd_device* usbd_dev, uint16_t wValue) {
    (void)wValue;

    usbd_ep_setup(usbd_dev, 0x03, USB_ENDPOINT_ATTR_BULK, 64, cdcacm_data_rx_cb);
    usbd_ep_setup(usbd_dev, 0x83, USB_ENDPOINT_ATTR_BULK, 64, NULL);
    usbd_ep_setup(usbd_dev, 0x84, USB_ENDPOINT_ATTR_INTERRUPT, 16, NULL);

    usbd_register_control_callback(
        usbd_dev, USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE,
        USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT, cdcacm_control_request);
}
