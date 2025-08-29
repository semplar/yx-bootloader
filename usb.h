#pragma once

/** STM32-specific USB endpoint record, addressable directly in PMA */
struct usb_ep_pma {
	uint16_t addr_tx;
	uint16_t _pma_hi0;
	uint16_t count_tx;
	uint16_t _pma_hi1;
	uint16_t addr_rx;
	uint16_t _pma_hi2;
	uint16_t count_rx;
	uint16_t _pma_hi3;
} __attribute__((packed));

/** For usb_ep_pma.count_rx */
enum PMARXCOUNT_BLSIZE {
	PMARXCOUNT_BLSIZE_2 = 0x0000,
	PMARXCOUNT_BLSIZE_32 = 0x8000,
};

#define PMARXCOUNT_NUM_BLOCK_SHIFT  10
#define PMARXCOUNT_NBYTES_MASK  0x3ff

/** control packet received during USB enumeration */
struct usb_setup_data_pma {
	uint8_t bmRequestType;
	uint8_t bRequest;
	uint16_t _pma_hi0;
	uint16_t wValue;
	uint16_t _pma_hi1;
	uint16_t wIndex;
	uint16_t _pma_hi2;
	uint16_t wLength;
	uint16_t _pma_hi3;
} __attribute__((packed));

/** Enables USB peripheral after device power-on. */
void usb_poweron();
/** Resets USB devices. Can be called manually, also can be initiated by USB host. */
void usb_reset();

/** Defined in USB 2.0 spec Section 9. */
struct usb_device_descriptor {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t bcdUSB;
	uint8_t bDeviceClass;
	uint8_t bDeviceSubClass;
	uint8_t bDeviceProtocol;
	uint8_t bMaxPacketSize0;
	uint16_t idVendor;
	uint16_t idProduct;
	uint16_t bcdDevice;
	uint8_t iManufacturer;
	uint8_t iProduct;
	uint8_t iSerialNumber;
	uint8_t bNumConfigurations;
} __attribute__((packed));

/** Defined in USB 2.0 spec Section 9. */
struct usb_config_descriptor {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t wTotalLength;
	uint8_t bNumInterfaces;
	uint8_t bConfigurationValue;
	uint8_t iConfiguration;
	uint8_t bmAttributes;
	uint8_t bMaxPower;
} __attribute__((packed));

/** Defined in USB 2.0 spec Section 9. */
struct usb_interface_descriptor {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bInterfaceNumber;
	uint8_t bAlternateSetting;
	uint8_t bNumEndpoints;
	uint8_t bInterfaceClass;
	uint8_t bInterfaceSubClass;
	uint8_t bInterfaceProtocol;
	uint8_t iInterface;
} __attribute__((packed));

/** Defined in USB 2.0 spec Section 9. */
struct usb_endpoint_descriptor {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bEndpointAddress;
	uint8_t bmAttributes;
	uint16_t wMaxPacketSize;
	uint8_t bInterval;
} __attribute__((packed));

/** Defined in USB 2.0 spec Section 9. */
struct usb_device_capability_descriptor {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDevCapabilityType;
} __attribute__((packed));

/** Defined in USB 2.0 spec Section 9. */
struct usb_bos_descriptor {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t wTotalLength;
	uint8_t bNumDeviceCaps;
} __attribute__((packed));

/** Defined in USB 2.0 spec Section 9. */
enum USB_REQUEST {
	REQUEST_GET_STATUS = 0,
	REQUEST_CLEAR_FEATURE = 1,
	REQUEST_SET_FEATURE = 3,
	REQUEST_SET_ADDRESS = 5,
	REQUEST_GET_DESCRIPTOR = 6,
	REQUEST_SET_DESCRIPTOR = 7,
	REQUEST_GET_CONFIGURATION = 8,
	REQUEST_SET_CONFIGURATION = 9,
	REQUEST_GET_INTERFACE = 10,
	REQUEST_SET_INTERFACE = 11,
	REQUEST_SYNCH_FRAME = 12,
};

/** Defined in USB 2.0 spec Section 9. */
enum USB_DESCRIPTOR {
	DESCRIPTOR_DEVICE = 1,
	DESCRIPTOR_CONFIGURATION = 2,
	DESCRIPTOR_STRING = 3,
	DESCRIPTOR_INTERFACE = 4,
	DESCRIPTOR_ENDPOINT = 5,
	DESCRIPTOR_DEVICE_QUALIFIER = 6,
	DESCRIPTOR_OTHER_SPEED_CONFIGURATION = 7,
	DESCRIPTOR_INTERFACE_POWER = 8,
};

