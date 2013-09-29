/*
 * Copyright 2012, Mark Fassler
 *
 *  Convenience functions for talking to the Peripheral Clock Selection registers.
 *
 * Peripherals take the systemClockClock (typically 100 MHz or so), and divide
 * that by 1, 2, 4, or 8.  The default divisor for all peripherals is divide-by-4
 *  (except for the RTC which is permanently at divide-by-8)
 */


#include <LPC17xx.h>
#include <type.h>

#include "peripheralClocks.h"

// TODO:
//  setPeripheralClock(int devNumber, int divisor)


uint32_t getPeripheralClock(int devNumber)
{
	uint32_t pclkdiv, pclk;

	switch (devNumber)
	{
		case PCLK_TIMER0:
		  pclkdiv = (LPC_SC->PCLKSEL0 >> 2) & 0x03;
		  break;
		case PCLK_TIMER1:
		  pclkdiv = (LPC_SC->PCLKSEL0 >> 4) & 0x03;
		  break;
		case PCLK_UART0:
		  pclkdiv = (LPC_SC->PCLKSEL0 >> 6) & 0x03;
		  break;
		case PCLK_UART1:
		  pclkdiv = (LPC_SC->PCLKSEL0 >> 8) & 0x03;
		  break;
		case PCLK_PWM1:
		  pclkdiv = (LPC_SC->PCLKSEL0 >> 12) & 0x03;
		  break;
		case PCLK_SSP1:
		  pclkdiv = (LPC_SC->PCLKSEL0 >> 20) & 0x03;
		  break;
		case PCLK_ADC:
		  pclkdiv = (LPC_SC->PCLKSEL0 >> 24) & 0x03;
		  break;
		case PCLK_SSP0:
		  pclkdiv = (LPC_SC->PCLKSEL1 >> 10) & 0x03;
		  break;
		case PCLK_UART2:
		  pclkdiv = (LPC_SC->PCLKSEL1 >> 16) & 0x03;
		  break;
		default:
		  pclkdiv = 0;
		  break;
	}

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

	return pclk;
}

