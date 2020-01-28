#include <stdlib.h>

#include <libopencm3/cm3/scb.h>

#include "usb.h"

usbd_device* usbdDevice;

const struct usb_device_descriptor dev_descr = {
    .bLength = USB_DT_DEVICE_SIZE,
    .bDescriptorType = USB_DT_DEVICE,
    .bcdUSB = 0x0110,
    .bDeviceClass = 0xEF,
    .bDeviceSubClass = 2,
    .bDeviceProtocol = 1,
    .bMaxPacketSize0 = 64,
    .idVendor = 0x0483,
    .idProduct = 0x5741,
    .bcdDevice = 0x0110,
    .iManufacturer = 1,
    .iProduct = 2,
    .iSerialNumber = 3,
    .bNumConfigurations = 1,
};

extern const struct usb_iface_assoc_descriptor iadcdc_descr[];
extern const struct usb_interface_descriptor comm_iface[];
extern const struct usb_interface_descriptor data_iface[];

// MSD descriptors
extern const struct usb_iface_assoc_descriptor iadmsc_descr[];
extern const struct usb_endpoint_descriptor msc_endp[];
extern const struct usb_interface_descriptor msc_iface[];

// Join all interfaces
static const struct usb_interface ifaces[] = {
	// MSD interface
    {
        .num_altsetting = 1,
        .altsetting = msc_iface,
    },
	// CDC ACM interface
    {
        .num_altsetting = 1,
        .iface_assoc = iadcdc_descr,
        .altsetting = comm_iface,
    },
    {
        .num_altsetting = 1,
        .altsetting = data_iface,
    }
};

//
const struct usb_config_descriptor config_descr = {
    .bLength = USB_DT_CONFIGURATION_SIZE,
    .bDescriptorType = USB_DT_CONFIGURATION,
    .wTotalLength = 0,
    .bNumInterfaces = 3,
    .bConfigurationValue = 1,
    .iConfiguration = 0,
    .bmAttributes = 0xC0,
    .bMaxPower = 0x32,
    .interface = ifaces,
};

// Visual description
const char* usb_strings[] = {
    "Amperka", // supplier
    "IskraJS", // product
    "Loader", // function 
};
