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
	
	for (i = 0; i < 5; ++i) {
		P1_4 = 1;
		delay_ms(500);
		P1_4 = 0;
		delay_ms(500);
	}
	
	USB_CTRL |= (1 << 5);
	
	while (1) {
		P1 |= (1 << 4);
		delay_ms(100);
		P1 &= ~(1 << 4);
		delay_ms(100);
	}
}
