
#include "LPC17xx.h"
#include "type.h"
#include "bitTypes.h"
#include "dac.h"

void DACInit(void) {

	// Set p0.26 (pin 18 on the mbed) to be DAC output:
	LPC_PINCON->PINSEL1 &= ~0x00300000;
	LPC_PINCON->PINSEL1 |= 0x00200000;

	LPC_DAC->DACCNTVAL = 0xff;
	LPC_DAC->DACCTRL = bit1 | bit2;
	return;
}

