/*
 * Copyright 2012, Mark Fassler
 *
 */


#include <LPC17xx.h>
#include <type.h>

#include "peripheralClocks.h"

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
		case PCLK_ADC:
		  pclkdiv = (LPC_SC->PCLKSEL0 >> 24) & 0x03;
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

