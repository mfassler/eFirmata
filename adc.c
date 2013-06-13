/*
 *  Copyright 2012, Mark Fassler
 *  Licensed under the GPLv3
 *
 */


#include <LPC17xx.h>
#include <type.h>
#include "peripheralClocks.h"

#include "adc.h"
#include "debug.h"

#include "emac.h"
#include "network/MAC_ADDRESSES.h"
#include "network/firmataProtocol.h"

volatile uint16_t ADCValue[ADC_NUM];

extern struct bigEtherFrame *bigEtherFrameA;
extern struct bigEtherFrame *bigEtherFrameB;

volatile uint16_t whichFrame;
volatile uint16_t whichByteInPayload;

void ADC_IRQHandler (void) 
{
	uint32_t regVal;

	static uint32_t fourSamples[ADC_NUM][4];
	static uint8_t whichSample = 0;

	static struct bigEtherFrame *aFrame;
	if (whichFrame == 0)
	{
		aFrame = bigEtherFrameA;
	}
	else
	{
		aFrame = bigEtherFrameB;
	}

	regVal = LPC_ADC->ADSTAT;   /* Read ADC will clear the interrupt */

	if (regVal & (1<<4) )
	{
		ADCValue[4] = LPC_ADC->ADDR4 & 0xFFF0;
	}

	if ( regVal & (1<<5) )
	{
		fourSamples[5][whichSample] = LPC_ADC->ADDR5 & 0xFFF0;
		whichSample++;

		if(whichSample == 4)
		{
			whichSample = 0;

			// the right-shift by 2 is the same as divide by 4
			ADCValue[5] = (fourSamples[5][0] +fourSamples[5][1] + fourSamples[5][2] + fourSamples[5][3]) >> 2;

			aFrame->data[whichByteInPayload] = ADCValue[5];
			whichByteInPayload++;

			if (whichByteInPayload == 256)
			{
				whichByteInPayload = 0;
				if (whichFrame == 0)
				{
					whichFrame = 1;
					ethernetPleaseSend(1, sizeof(struct bigEtherFrame));
				}
				else
				{
					whichFrame = 0;
					ethernetPleaseSend(2, sizeof(struct bigEtherFrame));
				}
			}
		}
	}

	return;
}


void ADCInit(void)
{
	uint32_t i;
	uint32_t pclk;
	uint8_t clkdiv;

	whichFrame = 0;
	whichByteInPayload = 0;

	for ( i = 0; i < ADC_NUM; i++ )
	{
		ADCValue[i] = 0x0;
	}

	// Enable ADC controller:
	LPC_SC->PCONP |= (1 << 12);

	// We're only going to use two ADC pins: 
	//	P1.30 == AD0.4 (pin 19 on the mbed)
	//	P1.31 == AD0.5 (pin 20 on the mbed)
	LPC_PINCON->PINSEL3 |= 0xF0000000;  // AD0.4 and AD0.5
	//LPC_PINCON->PINSEL3 |= 0xC0000000;  // AD0.5 only

	// No pull-up no pull-down (function 10) on these ADC pins:
	LPC_PINCON->PINMODE3 &= ~0xf0000000;  // AD0.4 and AD0.5
	LPC_PINCON->PINMODE3 |= 0xa0000000;  // AD0.4 and AD0.5
	//LPC_PINCON->PINMODE3 &= ~0xC0000000;  // AD0.5 only
	//LPC_PINCON->PINMODE3 |= 0x80000000;  // AD0.5 only


	// The ADC clock rate is:   pclk / (clkdiv + 1)
	// We're aiming for slightly less than 13 MHz.
	// It takes 65 clocks for one sample, so the maximum
	// sample rate is 13MHz / 65 clocks = 200000 SPS.
	// There's only one real ADC, but the inputs are multiplexed,
	// so the sample rate is divided amongst each input.  
	pclk = getPeripheralClock(PCLK_ADC);
	debugLong("ADC's pclk: ", pclk);
	// ** Example:
	//  clkdiv = (pclk / SAMPLE_RATE - 1)

	// With two channels and my divide-by-four trick, this ends ups being very close
	// to 24400 SPS.  Not sure why:
	clkdiv = 1;

	LPC_ADC->ADCR = (1<<4) | (1<<5) // Enable channels 4 and 5
		| ( clkdiv << 8 )
		| ( 1 << 16 )   // BURST
		| ( 1 << 21 )   // PDN = 1, normal operation
		| ( 0 << 24 )   // START = 0 A/D conversion stops
		| ( 0 << 27 );  // EDGE = 0 (CAP/MAT singal falling,trigger A/D conversion)

	NVIC_EnableIRQ(ADC_IRQn);

	LPC_ADC->ADINTEN = (1<<4) | (1<<5);  // Enable interrupts for channels 4 and 5

	return;
}

