#include "ch552.h"
#include <stdint.h>

void delay_ms(uint16_t u16Delay);

void delay_ms(uint16_t u16Delay)
{
	
	while (u16Delay) {
		TF0 = 0;
		TH0 = 0xA2;
		TL0 = 0x40;
		TR0 = 1;
		while (!TF0) {
		}
		TR0 = 0;
		--u16Delay;
	}
}

SBIT(P1_4,0x90,4);

__xdata uint8_t u8Buff[64];

void send(uint8_t u8Data);

void send(uint8_t u8Data)
{
	uint8_t i;
	
	for (i = 0; i < 8; ++i) {
		if (u8Data & 0x80) {
			P1_4 = 1;
			delay_ms(4);
			P1_4 = 0;
			delay_ms(1);
		} else {
			P1_4 = 1;
			delay_ms(1);
			P1_4 = 0;
			delay_ms(4);
		}
		u8Data <<= 1;
	}
}

const uint8_t u8DeviceDescriptor[] = {
	0x12, /* 0 */	//Size of the Descriptor in Bytes 0x12 (18 bytes)
	0x01, /* 1 */	//Device Descriptor (0x01)
	0x00, /* 2 */	//USB Specification Number which device complies too. 0xJJMN USB2.0 0x0200, USB1.1 0x0110, USB1.0 0x0100
	0x00, /* 3 */	//USB Specification Number which device complies too. 0xJJMN USB2.0 0x0200, USB1.1 0x0110, USB1.0 0x0100
	0xff, /* 4 */	//Class Code: HID, CDC...
	0x00, /* 5 */	//Subclass Code (Assigned by USB Org)
	0x00, /* 6 */	//Protocol Code (Assigned by USB Org)
	0x40, /* 7 */	//Maximum Packet Size for Zero Endpoint. Valid Sizes are 8, 16, 32, 64. 0x40 64 bytes.
	0x34, /* 8 */	//Vendor ID (Assigned by USB Org) Low
	0x12, /* 9 */	//Vendor ID (Assigned by USB Org) High
	0x78, /* 10 */	//Product ID (Assigned by Manufacturer) Low
	0x56, /* 11 */	//Product ID (Assigned by Manufacturer) High
	0x00, /* 12 */	//Device Release Number BCD
	0x00, /* 13 */	//Device Release Number BCD
	0x00, /* 14 */	//Index of Manufacturer String Descriptor
	0x00, /* 15 */	//Index of Product String Descriptor
	0x00, /* 16 */	//Index of Serial Number String Descriptor
	0x01 /* 17 */	//Number of Possible Configurations
};

void main(void)
{
	uint8_t i;
	
	/* clock */
	SAFE_MOD = 0x55;
	SAFE_MOD = 0xAA;
	CLOCK_CFG = 0x86;
	SAFE_MOD = 0x00;
	
	/* P1.4 */
	/* Push-pull */
	P1_MOD_OC &= ~(1 << 4);
	P1_DIR_PU |= (1 << 4);
	
	P1 &= ~(1 << 4);
	
	T2MOD |= (1 << 7);
	T2MOD |= (1 << 4);
	TMOD = 0x01;
	
	USB_CTRL = (1 << 5) | (1 << 3) | (1 << 0);
	UDEV_CTRL |= (1 << 0);
	
	while (1) {
		if (UIF_BUS_RST) {
			P1_4 = 1;
			P1_4 = 0;
			UEP0_DMA = (uint16_t)u8Buff;
			UEP0_CTRL = 0x02;
			UIF_BUS_RST = 0;
		}
		if (UIF_TRANSFER) {
			//Check transaction on Endpoint 0
			if ((USB_INT_ST & 0x0F) == 0x00) {
				if (u8Buff[0] & 0x80) {
					/* device to host */
					P1_4 = 1;
					P1_4 = 0;
					P1_4 = 1;
					P1_4 = 0;
					switch (u8Buff[1]) {
						case 0x06:
							/* Get descriptor */
							switch (u8Buff[3]) {
								case 0x01:
									/* device descriptor */
									if (u8Buff[6] >= 0x12) {
										for (i = 0; i < 0x12; ++i) {
											u8Buff[i] = u8DeviceDescriptor[i];
										}
										UEP0_T_LEN = 0x12;
										UEP0_CTRL = 0x80 | 0x40	//Send as DATA1. Receive DATA1.
									} else {
										for (i = 0; i < u8Buff[6]; ++i) {
											u8Buff[i] = u8DeviceDescriptor[i];
										}
										UEP0_T_LEN = u8Buff[6];
										UEP0_CTRL = 0x40;	//Send as DATA1
									}
									break;
							}
							break;
					}
				} else {
					/* host to device */
					P1_4 = 1;
					P1_4 = 0;
					P1_4 = 1;
					P1_4 = 0;
					P1_4 = 1;
					P1_4 = 0;
					P1_4 = 1;
					P1_4 = 0;
				}
			} else {
			}
			UIF_TRANSFER = 0;
		}
	}
}
