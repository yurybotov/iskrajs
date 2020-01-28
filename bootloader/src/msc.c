#include <stdlib.h>
#include "usb.h"

// MSC end-point descriptor
const struct usb_endpoint_descriptor msc_endp[] = {
    { .bLength = USB_DT_ENDPOINT_SIZE,
        .bDescriptorType = USB_DT_ENDPOINT,
        .bEndpointAddress = 0x01,
        .bmAttributes = USB_ENDPOINT_ATTR_BULK,
        .wMaxPacketSize = 64,
        .bInterval = 0 
	},
    { .bLength = USB_DT_ENDPOINT_SIZE,
        .bDescriptorType = USB_DT_ENDPOINT,
        .bEndpointAddress = 0x81,
        .bmAttributes = USB_ENDPOINT_ATTR_BULK,
        .wMaxPacketSize = 64,
        .bInterval = 0 
	}
};

// MSC interface descriptor
const struct usb_interface_descriptor msc_iface[] = { { 
	.bLength = USB_DT_INTERFACE_SIZE,
    .bDescriptorType = USB_DT_INTERFACE,
    .bInterfaceNumber = 0,
    .bAlternateSetting = 0,
    .bNumEndpoints = 2,
    .bInterfaceClass = USB_CLASS_MSC,
    .bInterfaceSubClass = USB_MSC_SUBCLASS_SCSI,
    .bInterfaceProtocol = USB_MSC_PROTOCOL_BBB,
    .iInterface = 0,
    .endpoint = msc_endp,
    .extra = NULL,
    .extralen = 0 
} };
