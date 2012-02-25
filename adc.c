/****************************************************************************
 *   $Id:: adc.c 6089 2011-01-06 04:38:09Z nxp12832                         $
 *   Project: NXP LPC17xx ADC example
 *
 *   Description:
 *     This file contains ADC code example which include ADC 
 *     initialization, ADC interrupt handler, and APIs for ADC
 *     reading.
 *
 ****************************************************************************
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * products. This software is supplied "AS IS" without any warranties.
 * NXP Semiconductors assumes no responsibility or liability for the
 * use of the software, conveys no license or title under any patent,
 * copyright, or mask work right to the product. NXP Semiconductors
 * reserves the right to make changes in the software without
 * notification. NXP Semiconductors also make no representation or
 * warranty that such application will be suitable for the specified
 * use without further testing or modification.
****************************************************************************/
#include <LPC17xx.h>
#include "type.h"
#include "adc.h"
#include "debug.h"
#include "emac.h"

volatile uint16_t ADCValue[ADC_NUM];
volatile uint32_t ADCIntDone = 0;
volatile uint32_t BurstCounter = 0;
volatile uint32_t OverRunCounter = 0;

volatile uint16_t whichFrame;
volatile uint16_t whichByteInPayload;

extern char bigEtherFrame[2][6+6+2+1024+4];

void ADC_IRQHandler (void) 
{
    uint32_t regVal;
    //volatile uint32_t dummy;
    uint16_t previousFrame;
    uint16_t frameIdx;
    //int i;

//    debug("inside ADC_IRQHandler()");

    regVal = LPC_ADC->ADSTAT;   /* Read ADC will clear the interrupt */

/*
    if ( regVal & 0x0000FF00 )	// check for overrun
    {
        OverRunCounter++;
//        for (i = 0; i < ADC_NUM; i++)
//        {
            regVal = (regVal & 0x0000FF00) >> 0x08;
            // if overrun, just read ADDR to clear 
            // regVal variable has been reused. 
            if ( regVal & (1<<5) )
            {
                dummy = LPC_ADC->ADDR5;
            }
//        }
        LPC_ADC->ADCR &= ~((0x7<<24)|(0x1<<16));	// stop ADC now, turn off BURST bit. 
        ADCIntDone = 1;
        return;	
    }
*/


    if ( regVal & (1<<5) )
    {
        ADCValue[5] = ( LPC_ADC->ADDR5 >> 4 ) & 0xFFF;
//        debugLong("ADCValue[5]: ", ADCValue[5]);

        frameIdx = whichByteInPayload + 6 + 6 + 2;
        bigEtherFrame[whichFrame][frameIdx] = (ADCValue[5] & 0xFF0 ) >> 4;
//        whichByteInPayload++;


/*        if (whichByteInPayload == 256)
        {
            whichByteInPayload = 0;
            previousFrame = whichFrame;
            whichFrame++;
            if (whichFrame == 2)
            {
                whichFrame = 0;
            }
            RequestSend(6 + 6+ 2 + 256 + 4);
            CopyToFrame_EMAC( (unsigned short*) bigEtherFrame[previousFrame], 6+6+2+256+4);
        }*/
    }



//    LPC_ADC->ADCR &= ~(0x7<<24);    /* stop ADC now */ 
//    ADCIntDone = 1;

//    ADCRead(5);
    return;
}


void ADCInit(void)
{
    uint32_t i, pclkdiv, pclk;

    whichFrame = 0;
    whichByteInPayload = 0;

    for ( i = 0; i < ADC_NUM; i++ )
    {
        ADCValue[i] = 0x0;
    }

    /* Enable CLOCK into ADC controller */
    LPC_SC->PCONP |= (1 << 12);

    // We're only going to use one ADC pin:  P1.31 == AD0.5
    // (pin 20 on the mbed)
    LPC_PINCON->PINSEL3 |= 0xC0000000;	/* P1.31, AD0.5, function 11 */

    /* No pull-up no pull-down (function 10) on these ADC pins. */
    LPC_PINCON->PINMODE3 &= ~0xC0000000;
    LPC_PINCON->PINMODE3 |= 0x80000000;


    /* By default, the PCLKSELx value is zero, thus, the PCLK for
    all the peripherals is 1/4 of the SystemFrequency. */
    /* Bit 24~25 is for ADC */
    pclkdiv = (LPC_SC->PCLKSEL0 >> 24) & 0x03;
    switch (pclkdiv)
    {
	    case 0x00:
    	default:
	      pclk = SystemCoreClock/4;
          break;
    	case 0x01:
          pclk = SystemCoreClock;
          break; 
        case 0x02:
          pclk = SystemCoreClock/2;
          break; 
        case 0x03:
          pclk = SystemCoreClock/8;
          break;
    }

    debugLong("pclk: ", pclk);
    debugLong("SystemCoreClock: ", SystemCoreClock);
    //debugLong("ADC_Clk: ", ADC_Clk);

    LPC_ADC->ADCR = ( 0x01 << 5 ) |  /* SEL=1,select channel 0~7 on ADC0 */
    //    ( ( pclk  / ADC_Clk - 1 ) << 8 ) |  /* CLKDIV = Fpclk / ADC_Clk - 1 */ 
        ( 9 << 8 ) |   // gives a sample rate of 38461 samples per second
        ( 1 << 16 ) | 		/* BURST */
        ( 1 << 21 ) |  		/* PDN = 1, normal operation */
        ( 0 << 24 ) |  		/* START = 0 A/D conversion stops */
        ( 0 << 27 );		/* EDGE = 0 (CAP/MAT singal falling,trigger A/D conversion) */ 

    NVIC_EnableIRQ(ADC_IRQn);

    LPC_ADC->ADINTEN = (1 << 5);  /* Enable interrupts for channel 5 */

    return;
}

/*****************************************************************************
** Function name:		ADCRead
**
** Descriptions:		Read ADC channel
**
** parameters:			Channel number
** Returned value:		Value read, if interrupt driven, return channel #
** 
*****************************************************************************/
uint32_t ADCRead( uint8_t channelNum )
{

    /* channel number is 0 through 7 */
    if ( channelNum >= ADC_NUM )
    {
        channelNum = 0;		/* reset channel number to 0 */
    }
    LPC_ADC->ADCR &= 0xFFFFFF00;
    LPC_ADC->ADCR |= (1 << 24) | (1 << channelNum);	

    /* switch channel,start A/D convert */

    return ( channelNum );	/* if it's interrupt driven, the ADC reading is 
							done inside the handler. so, return channel number */
}

