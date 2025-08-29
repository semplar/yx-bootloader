#include <libopencm3/cm3/memorymap.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/st_usbfs.h>

#include "usb.h"
#include "debug.h"
#include "hardware.h"

_Alignas(2) const struct usb_device_descriptor DEV_DESC = {
	.bLength = 18,
	.bDescriptorType = DESCRIPTOR_DEVICE,
	.bcdUSB = 0x0200,        // USB 2.0
	.bDeviceClass = 0x00,    // see interface
	.bDeviceSubClass = 0x00, // see interface
	.bDeviceProtocol = 0x00, // see interface
	.bMaxPacketSize0 = 64,   // 64 bytes
	.idVendor = 0x0483,
	.idProduct = 0x0057,
	.bcdDevice = 0x0001,
	.iManufacturer = 0x01,
	.iProduct = 0x02,
	.iSerialNumber = 0x03,
	.bNumConfigurations = 1,
};

_Alignas(2) const struct usb_config_descriptor CONF_DESC = {
	.bLength = 9,
	.bDescriptorType = DESCRIPTOR_CONFIGURATION,
	.wTotalLength = 0x0000, // to be filled later
	.bNumInterfaces = 1,
	.bConfigurationValue = 1,
	.iConfiguration = 0,
	.bmAttributes = 0x80, // bus powered
	.bMaxPower = 50,      // 100mA
};

_Alignas(2) const struct usb_interface_descriptor IFACE_DESC = {
	.bLength = 9,
	.bDescriptorType = DESCRIPTOR_INTERFACE,
	.bInterfaceNumber = 0,
	.bAlternateSetting = 0,
	.bNumEndpoints = 0,
	.bInterfaceClass = 0xff,
	.bInterfaceSubClass = 0x00,
	.bInterfaceProtocol = 0x00,
	.iInterface = 0,
};

#define PMA_BTABLE  0x000
#define PMA_EP0_TX  0x040
#define PMA_EP0_RX  0x080

int8_t device_address_change = 0;

// assuming BTABLE = 0x000
static inline void* pma_addr(uint16_t offset) {
	return (void*) (USB_PMA_BASE + (((offset & ~1) << 1) | (offset & 1)));
}

#define NVIC_ISER(iser_id)		MMIO32(NVIC_BASE + 0x00 + ((iser_id) * 4))

// high-priority interrupt. used for isochronous and double-buffered transfers
void usb_isr_hp() {
	debugc('H');
	// TODO: not implemented yet
}

// copies 2-byte aligned buffer into PMA memory
void copy16_to_pma(uint16_t pma_offset, uint16_t* source, uint16_t nbytes) {
	uint16_t* source_end = source + (nbytes >> 1);
	uint16_t* pma_dst = (uint16_t*) pma_addr(pma_offset & ~1);
	while (source < source_end) {
		*pma_dst++ = *source++;
		pma_dst++;
	}
	if (nbytes & 1)
		*(char*) pma_dst = *(char*) source_end;
}

// prints bytes from PMA memory to debug console
void debug_pma(uint16_t pma_offset, uint16_t nbytes) {
	uint8_t* rx_buf_ptr = (uint8_t*) pma_addr(pma_offset);
	for (unsigned int i = 0; i < nbytes; i++) {
		debugh(*rx_buf_ptr, 8);
		rx_buf_ptr += 1 + (((uint32_t) rx_buf_ptr & 1) << 1);
	}
}

void usb_isr_lp() {
    uint16_t int_status = *USB_ISTR_REG;
//	do {
		if (int_status & USB_ISTR_RESET) { // USB reset
			debugs("<R>");
			*USB_ISTR_REG = ~USB_ISTR_RESET;
			usb_reset();
		}
		if (int_status & USB_ISTR_PMAOVR) { // PMA overrun / underrun
			debugs("<O>");
			*USB_ISTR_REG = ~USB_ISTR_PMAOVR;
			// TODO
		}
		if (int_status & USB_ISTR_ERR) { // NANS, CRC, BST, FVIO
			debugs("<E>");
			*USB_ISTR_REG = ~USB_ISTR_ERR;
			// TODO
		}
		if (int_status & USB_ISTR_WKUP) {
			debugs("<W>");
			*USB_ISTR_REG = ~USB_ISTR_WKUP;
//			usbResume(RESUME_EXTERNAL);
		}
		if (int_status & USB_ISTR_SUSP) {
//			debugc('S');
			*USB_ISTR_REG = ~USB_ISTR_SUSP;
			// TODO: not supported yet
		}
		if (int_status & USB_ISTR_SOF) {
			debugc('~');
			*USB_ISTR_REG = ~USB_ISTR_SOF;
//			bIntPackSOF++;
		}
		if (int_status & USB_ISTR_ESOF) {
//			debugc('G');
			*USB_ISTR_REG = ~USB_ISTR_ESOF;
			// TODO: not supported yet
		}
		if (int_status & USB_ISTR_CTR) {
			uint8_t ep_id = int_status & USB_ISTR_EP_ID;
			// uint8_t ep_dir = int_status & USB_ISTR_DIR;
			volatile uint32_t* ep_reg = USB_EP_REG(ep_id);
			uint16_t ep_status = *ep_reg;
			debugc('@'); debugc('0' + ep_id); debugc(':'); debugh(ep_status, 16);
			uint16_t ep_out = ep_status & USB_EP_NTOGGLE_MSK;
			if (ep_status & USB_EP_RX_CTR) {
				uint16_t is_setup = ep_status & USB_EP_SETUP;
				debugc('<');
				if (is_setup)
					debugc('?');
				struct usb_ep_pma* epregs = (struct usb_ep_pma*) pma_addr(PMA_BTABLE + ep_id * 8);
				uint16_t recv_nbytes = epregs->count_rx & PMARXCOUNT_NBYTES_MASK;
				debug_pma(PMA_EP0_RX, recv_nbytes);
				if (ep_id == 0 && is_setup) {
					if (recv_nbytes == 0) { // status out
						ep_out ^= (ep_status & USB_EP_RX_STAT) ^ USB_EP_RX_STAT_VALID;
						ep_out ^= (ep_status & USB_EP_TX_STAT) ^ USB_EP_TX_STAT_NAK;
					} else {
						struct usb_setup_data_pma* packet = (struct usb_setup_data_pma*) pma_addr(PMA_EP0_RX);
						switch (packet->bRequest) {
							case REQUEST_GET_DESCRIPTOR: {
								// respond with a descriptor
								copy16_to_pma(PMA_EP0_TX, (uint16_t*) &DEV_DESC, sizeof(DEV_DESC));
								epregs->addr_tx = PMA_EP0_TX;
								epregs->count_tx = sizeof(DEV_DESC);
//								ep_out ^= (ep_status & USB_EP_RX_STAT) ^ USB_EP_RX_STAT_STALL;
								ep_out ^= (ep_status & USB_EP_TX_STAT) ^ USB_EP_TX_STAT_VALID;
								break;
							}
							case REQUEST_SET_ADDRESS: {
								// set device address
								device_address_change = packet->wValue;
								epregs->addr_tx = PMA_EP0_TX;
								epregs->count_tx = 0;
//								ep_out ^= (ep_status & USB_EP_RX_STAT) ^ USB_EP_RX_STAT_VALID;
								ep_out ^= (ep_status & USB_EP_TX_STAT) ^ USB_EP_TX_STAT_VALID;
								break;
							}
							default:
								// not supported yet: do not reply
								break;
						}
					}
				}
				ep_out &= ~USB_EP_RX_CTR;
			}
			if (ep_status & USB_EP_TX_CTR) {
				uint16_t is_setup = ep_status & USB_EP_SETUP;
				debugc('>');
				if (is_setup)
					debugc('?');
				struct usb_ep_pma* epregs = (struct usb_ep_pma*) pma_addr(PMA_BTABLE + ep_id * 8);
				debug_pma(PMA_EP0_TX, epregs->count_tx);
				ep_out ^= (ep_status & USB_EP_RX_STAT) ^ USB_EP_RX_STAT_VALID;
//				ep_out ^= (ep_status & USB_EP_TX_STAT) ^ USB_EP_TX_STAT_NAK;
				if (device_address_change) {
					debugc('A'); debugh(device_address_change, 8);
					*USB_DADDR_REG = (device_address_change & USB_DADDR_ADDR) | USB_DADDR_EF;
					device_address_change = 0;
				}
				ep_out &= ~USB_EP_TX_CTR;
			}
			debugc('_'); debugh(*ep_reg, 16);
			debugc('#'); debugh(ep_out, 16);
			*ep_reg = ep_out;
			debugc('='); debugh(*ep_reg, 16);
		}
		debugc('\n');
//		break;
//		int_status = *USB_ISTR_REG;
//	} while (int_status & (USB_ISTR_RESET | USB_ISTR_PMAOVR | USB_ISTR_ERR | USB_ISTR_WKUP | USB_ISTR_SUSP | USB_ISTR_SOF | USB_ISTR_ESOF | USB_ISTR_CTR));
}

void usb_isr_wakeup() {
	debugc('^');
	// TODO: not implemented yet
}

void usb_poweron() {
	// assuming 72MHz system clock and 48MHz clock is already passed to USB

	// enable usb bus
	PREG_CLEAR_SET(&RCC_APB1ENR, 0, RCC_APB1ENR_USBEN);
	PREG_CLEAR_SET(&RCC_APB1RSTR, RCC_APB1RSTR_USBRST, 0);

	*USB_CNTR_REG = USB_CNTR_FRES; // PWDN=0: power up USB peripheral, FRES=1: keep it in reset state

	// wait 1us, TODO: replace by sleep
	for (unsigned int i = 0; i < 72000000 / 1000000; i++) {}

	isr_ram[0x10 + 19] = usb_isr_hp;
	isr_ram[0x10 + 20] = usb_isr_lp;
	isr_ram[0x10 + 42] = usb_isr_wakeup;

	NVIC_ISER(0) = (1 << 20) | (1 << 19); // enable usb lp (19) and hp (20) interrupts 
	NVIC_ISER(1) = (1 << 10);             // enable usb wakeup interrupt (42)

	*USB_BTABLE_REG = PMA_BTABLE; // buffer descriptor table is at the start of PMA
	struct usb_ep_pma* epregs = (struct usb_ep_pma*) pma_addr(PMA_BTABLE);
	epregs->addr_rx = PMA_EP0_RX;
	epregs->count_rx = PMARXCOUNT_BLSIZE_32 | (2 << PMARXCOUNT_NUM_BLOCK_SHIFT); // 64 bytes of RX buffer is available

	*USB_ISTR_REG = 0; // clear any interrupts if there were any
	*USB_CNTR_REG = USB_CNTR_CTRM | USB_CNTR_RESETM; // | USB_CNTR_WKUPM | USB_CNTR_SUSPM; // FRES=0: release reset state
}

void usb_reset() {
	volatile uint32_t* ep_reg = USB_EP_REG(0);
	uint16_t ep_status = *ep_reg;
	uint16_t ep_out = ep_status & USB_EP_NTOGGLE_MSK;
	ep_out = (ep_out & ~USB_EP_TYPE) | USB_EP_TYPE_CONTROL;
	ep_out ^= (ep_status & USB_EP_TX_STAT) ^ USB_EP_TX_STAT_NAK;
	ep_out ^= (ep_status & USB_EP_RX_STAT) ^ USB_EP_RX_STAT_VALID;
	debugc('_'); debugh(*ep_reg, 16); 
	debugc('#'); debugh(ep_out, 16);
    *ep_reg = ep_out; // USB_EP_KIND is cleared
	debugc('='); debugh(*ep_reg, 16);
	
	*USB_DADDR_REG = USB_DADDR_EF; // enable device function, address is 0
}
