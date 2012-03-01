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
#include "MAC_ADDRESSES.h"
#include "firmataProtocol.h"

volatile uint16_t ADCValue[ADC_NUM];

extern struct bigEtherFrame *bigEtherFrameA;
extern struct bigEtherFrame *bigEtherFrameB;

volatile uint16_t whichFrame;
volatile uint16_t whichByteInPayload;

void ADC_IRQHandler (void) 
{
    uint32_t regVal;

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
        ADCValue[5] = LPC_ADC->ADDR5 & 0xFFF0;

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

    return;
}


void ADCInit(void)
{
    uint32_t i, pclk;

    whichFrame = 0;
    whichByteInPayload = 0;

    for ( i = 0; i < ADC_NUM; i++ )
    {
        ADCValue[i] = 0x0;
    }

    /* Enable CLOCK into ADC controller */
    LPC_SC->PCONP |= (1 << 12);

    // We're only going to use two ADC pins: 
    //    P1.30 == AD0.4 (pin 19 on the mbed)
    //    P1.31 == AD0.5 (pin 20 on the mbed)
    LPC_PINCON->PINSEL3 |= 0xF0000000;  // AD0.4 and AD0.5
    //LPC_PINCON->PINSEL3 |= 0xC0000000;  // AD0.5 only

    // No pull-up no pull-down (function 10) on these ADC pins:
    LPC_PINCON->PINMODE3 &= ~0xf0000000;  // AD0.4 and AD0.5
    LPC_PINCON->PINMODE3 |= 0xa0000000;  // AD0.4 and AD0.5
    //LPC_PINCON->PINMODE3 &= ~0xC0000000;  // AD0.5 only
    //LPC_PINCON->PINMODE3 |= 0x80000000;  // AD0.5 only


    pclk = getPeripheralClock(PCLK_ADC);

    LPC_ADC->ADCR = (1<<4) | (1<<5) | // Enable channels 4 and 5
    //    ( ( pclk  / ADC_Clk - 1 ) << 8 ) |  /* CLKDIV = Fpclk / ADC_Clk - 1 */ 
        ( 5 << 8 ) |   // gives a sample rate of 38461 samples per second
        ( 1 << 16 ) | 		/* BURST */
        ( 1 << 21 ) |  		/* PDN = 1, normal operation */
        ( 0 << 24 ) |  		/* START = 0 A/D conversion stops */
        ( 0 << 27 );		/* EDGE = 0 (CAP/MAT singal falling,trigger A/D conversion) */ 

    NVIC_EnableIRQ(ADC_IRQn);

    LPC_ADC->ADINTEN = (1<<4) | (1<<5);  // Enable interrupts for channels 4 and 5

    return;
}

