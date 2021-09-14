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
	0x12, /* 0 */
	0x01, /* 1 */
	0x00, /* 2 */	//USB2.0
	0x02, /* 3 */	//USB2.0
	0xff, /* 4 */
	0x00, /* 5 */
	0x00, /* 6 */
	0x40, /* 7 */
	0x34, /* 8 */
	0x12, /* 9 */
	0x78, /* 10 */
	0x56, /* 11 */
	0x00, /* 12 */
	0x00, /* 13 */
	0x00, /* 14 */
	0x00, /* 15 */
	0x00, /* 16 */
	0x01 /* 17 */
};

const uint8_t u8ConfigDescriptor[] = {
	0x09,	//Size of Descriptor 9 bytes
	0x02,	//Configuration Descriptor (0x02)
	0x09,
	0x00,
	0x01,
	0x01,
	0x00,
	0xC0,
	0x32	//100mA=50units, one unit is 2mA 
};

uint8_t u8Address = 0;

#define DATA_STATE 0
#define STATUS_STATE 1

uint8_t u8ControlState = DATA_STATE;

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
			USB_DEV_AD = 0;
			u8Address = 0;
			UEP0_DMA = (uint16_t)u8Buff;
			UEP0_CTRL = 0x02;
			UIF_BUS_RST = 0;
		}
		if (UIF_TRANSFER) {
			P1_4 = 1;
			P1_4 = 0;
			if ((USB_INT_ST & 0x0F) == 0x00) {
				if ((USB_INT_ST & 0x30) == 0x30) {
					/* EP0 Setup */
					if (u8Buff[0] & 0x80) {
						/* device to host */
						switch (u8Buff[1]) {
							case 0x06:
								/* Get descriptor */
								switch (u8Buff[3]) {
									case 0x01:
										u8ControlState = DATA_STATE;
										/* device descriptor */
										if (u8Buff[6] >= 0x12) {
											for (i = 0; i < 0x12; ++i) {
												u8Buff[i] = u8DeviceDescriptor[i];
											}
											UEP0_T_LEN = 0x12;
											UEP0_CTRL = 0x80 | 0x40;
										}
										break;
									case 0x02:
										u8ControlState = DATA_STATE;
										/* config */
										if (u8Buff[6] >= 9) {
											for (i = 0; i < 9; ++i) {
												u8Buff[i] = u8ConfigDescriptor[i];
											}
											UEP0_T_LEN = 9;
											UEP0_CTRL = 0x80 | 0x40;
										}
										break;
									case 0x06:
										u8ControlState = DATA_STATE;
										UEP0_T_LEN = 0x00;
										UEP0_CTRL = 0x80 | 0x40;
										break;
								}
								break;
						}
					} else {
						/* host to device */
						switch (u8Buff[1]) {
							case 0x05:
								u8ControlState = STATUS_STATE;
								/* set address */
								u8Address = u8Buff[2];
								UEP0_T_LEN = 0;
								UEP0_CTRL = 0x40;
								break;
						}
					}
				} else if ((USB_INT_ST & 0x30) == 0x00) {
					/* EP0 out */
				} else if ((USB_INT_ST & 0x30) == 0x20) {
					if (u8ControlState == DATA_STATE) {
						u8ControlState = STATUS_STATE;
						UEP0_CTRL = 0x80 | 0x40 | 0x02;
					} else {
						/* EP0 in */
						if (u8Address) {
							USB_DEV_AD = u8Address;
							u8Address = 0;
						}
						UEP0_CTRL = 0x40 | 0x02;	//Receive Data is enabled, Send Data is disabled
					}
				} else {
					/* EP0 sof */
				}
			} else {
			}
			UIF_TRANSFER = 0;
		}
	}
}
