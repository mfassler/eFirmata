/****************************************************************************
 *   $Id:: uart.c 5751 2010-11-30 23:56:11Z usb00423						$
 *   Project: NXP LPC17xx UART example
 *
 *   Description:
 *	 This file contains UART code example which include UART initialization, 
 *	 UART interrupt handler, and APIs for UART access.
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
#include <type.h>
#include "peripheralClocks.h"
#include "uart.h"
#include "debug.h"

volatile uint32_t UART0Status, UART1Status;
volatile uint8_t UART0TxEmpty = 1, UART1TxEmpty = 1;
volatile uint8_t UART0Buffer[BUFSIZE], UART1Buffer[BUFSIZE];
volatile uint32_t UART0Count = 0, UART1Count = 0;

void UART0_IRQHandler (void) 
{
	uint8_t IIRValue, LSRValue;
	uint8_t Dummy = Dummy;

	IIRValue = LPC_UART0->IIR;
	
	IIRValue >>= 1; /* skip pending bit in IIR */
	IIRValue &= 0x07; /* check bit 1~3, interrupt identification */
	if (IIRValue == IIR_RLS)  /* Receive Line Status */
	{
		LSRValue = LPC_UART0->LSR;
		/* Receive Line Status */
		if ( LSRValue & (LSR_OE|LSR_PE|LSR_FE|LSR_RXFE|LSR_BI) )
		{
			/* There are errors or break interrupt */
			/* Read LSR will clear the interrupt */
			UART0Status = LSRValue;
			Dummy = LPC_UART0->RBR; /* Dummy read on RX to clear 
										interrupt, then bail out */
			return;
		}
		if ( LSRValue & LSR_RDR ) /* Receive Data Ready */			
		{
			/* If no error on RLS, normal ready, save into the data buffer. */
			/* Note: read RBR will clear the interrupt */
			UART0Buffer[UART0Count] = LPC_UART0->RBR;
			UART0Count++;
			if (UART0Count == BUFSIZE)
			{
				UART0Count = 0; /* buffer overflow */
			}	
		}
	}
	else if (IIRValue == IIR_RDA) /* Receive Data Available */
	{
		/* Receive Data Available */
		UART0Buffer[UART0Count] = LPC_UART0->RBR;
		UART0Count++;
		if (UART0Count == BUFSIZE)
		{
			UART0Count = 0; /* buffer overflow */
		}
	}
	else if (IIRValue == IIR_CTI) /* Character timeout indicator */
	{
		/* Character Time-out indicator */
		UART0Status |= 0x100; /* Bit 9 as the CTI error */
	}
	else if (IIRValue == IIR_THRE) /* THRE, transmit holding register empty */
	{
		/* THRE interrupt */
		LSRValue = LPC_UART0->LSR; /* Check status in the LSR to see if
									  valid data in U0THR or not */
		if (LSRValue & LSR_THRE)
		{
			UART0TxEmpty = 1;
		}
		else
		{
			UART0TxEmpty = 0;
		}
	}
}

void UART1_IRQHandler (void) 
{
	uint8_t IIRValue, LSRValue;
	uint8_t Dummy = Dummy;

	IIRValue = LPC_UART1->IIR;

	IIRValue >>= 1;  /* skip pending bit in IIR */
	IIRValue &= 0x07; /* check bit 1~3, interrupt identification */
	if (IIRValue == IIR_RLS)  /* Receive Line Status */
	{
		LSRValue = LPC_UART1->LSR;
		/* Receive Line Status */
		if ( LSRValue & (LSR_OE|LSR_PE|LSR_FE|LSR_RXFE|LSR_BI) )
		{
			/* There are errors or break interrupt */
			/* Read LSR will clear the interrupt */
			UART1Status = LSRValue;
			Dummy = LPC_UART1->RBR; /* Dummy read on RX to clear 
									   interrupt, then bail out */
			return;
		}
		if ( LSRValue & LSR_RDR ) /* Receive Data Ready */			
		{
			/* If no error on RLS, normal ready, save into the data buffer. */
			/* Note: read RBR will clear the interrupt */
			UART1Buffer[UART1Count] = LPC_UART1->RBR;
			UART1Count++;
			if (UART1Count == BUFSIZE)
			{
				UART1Count = 0;  /* buffer overflow */
			}	
		}
	}
	else if (IIRValue == IIR_RDA) /* Receive Data Available */
	{
		/* Receive Data Available */
		UART1Buffer[UART1Count] = LPC_UART1->RBR;
		UART1Count++;
		if (UART1Count == BUFSIZE)
		{
			UART1Count = 0; /* buffer overflow */
		}
	}
	else if (IIRValue == IIR_CTI) /* Character timeout indicator */
	{
		/* Character Time-out indicator */
		UART1Status |= 0x100;   /* Bit 9 as the CTI error */
	}
	else if (IIRValue == IIR_THRE) /* THRE, transmit holding register empty */
	{
		/* THRE interrupt */
		LSRValue = LPC_UART1->LSR;  /* Check status in the LSR to see if
									   valid data in U0THR or not */
		if ( LSRValue & LSR_THRE )
		{
			UART1TxEmpty = 1;
		}
		else
		{
			UART1TxEmpty = 0;
		}
	}
}

uint32_t UARTInit(uint32_t PortNum, uint32_t baudrate)
{
	uint32_t Fdiv;
	uint32_t pclk;

	if ( PortNum == 0 )
	{
		pclk = getPeripheralClock(PCLK_UART0);
		Fdiv = ( pclk / 16 ) / baudrate;
		LPC_PINCON->PINSEL0 &= ~0x000000F0;
		LPC_PINCON->PINSEL0 |= 0x00000050;  /* RXD0 on P0.3; TXD0 on P0.2 */

		LPC_UART0->LCR = 0x83;		/* 8 bits, no Parity, 1 Stop bit */
		LPC_UART0->DLM = Fdiv / 256;							
		LPC_UART0->DLL = Fdiv % 256;
		LPC_UART0->LCR = 0x03;  /* DLAB = 0 */
		LPC_UART0->FCR = 0x07;  /* Enable and reset TX and RX FIFO. */

//		NVIC_EnableIRQ(UART0_IRQn);
//		LPC_UART0->IER = IER_RBR | IER_THRE | IER_RLS;  /* Enable UART0 interrupt */
		return (TRUE);
	}
	else if ( PortNum == 1 )
	{
		pclk = getPeripheralClock(PCLK_UART1);
		Fdiv = ( pclk / 16 ) / baudrate;
		LPC_PINCON->PINSEL0 &= ~0xC0000000;
		LPC_PINCON->PINSEL0 |= 0x40000000;  /* TXD1 on P0.15 */
		LPC_PINCON->PINSEL1 &= ~0x00000003;
		LPC_PINCON->PINSEL1 |= 0x00000001;  /* RXD1 on P0.16 */

		LPC_UART1->LCR = 0x83;		/* 8 bits, no Parity, 1 Stop bit */
		LPC_UART1->DLM = Fdiv / 256;							
		LPC_UART1->DLL = Fdiv % 256;
		LPC_UART1->LCR = 0x03; /* DLAB = 0 */
		LPC_UART1->FCR = 0x07; /* Enable and reset TX and RX FIFO. */

//		NVIC_EnableIRQ(UART1_IRQn);
//		LPC_UART1->IER = IER_RBR | IER_THRE | IER_RLS;	/* Enable UART1 interrupt */
		return (TRUE);
	}
	else if ( PortNum == 2 )
	{
		pclk = getPeripheralClock(PCLK_UART2);
		Fdiv = ( pclk / 16 ) / baudrate;
		LPC_SC->PCONP |= (1 << 24);
		LPC_PINCON->PINSEL0 &= ~0x00F00000;
		LPC_PINCON->PINSEL0 |= 0x00500000;  /* RXD2 on P0.11; TXD2 on P0.10 */

		LPC_UART2->LCR = 0x83;		/* 8 bits, no Parity, 1 Stop bit */
		LPC_UART2->DLM = Fdiv / 256;							
		LPC_UART2->DLL = Fdiv % 256;
		LPC_UART2->LCR = 0x03; /* DLAB = 0 */
		LPC_UART2->FCR = 0x07; /* Enable and reset TX and RX FIFO. */

//		NVIC_EnableIRQ(UART2_IRQn);
//		LPC_UART2->IER = IER_RBR | IER_THRE | IER_RLS;	/* Enable UART2 interrupt */
		return (TRUE);
	}

	return(FALSE); 
}

void UARTSend(uint32_t portNum, uint8_t *BufferPtr, uint32_t Length)
{
	if (portNum == 0)
	{
		while ( Length != 0 )
		{
			/* THRE status, contain valid data */
			while ( !(UART0TxEmpty & 0x01) );	
			LPC_UART0->THR = *BufferPtr;
			UART0TxEmpty = 0; /* not empty in the THR until it shifts out */
			BufferPtr++;
			Length--;
		}
	}
	else if (portNum == 1)
	{
		while (Length != 0)
		{
			/* THRE status, contain valid data */
			while ( !(UART1TxEmpty & 0x01) );	
			LPC_UART1->THR = *BufferPtr;
			UART1TxEmpty = 0; /* not empty in the THR until it shifts out */
			BufferPtr++;
			Length--;
		}
	}
/*	else if (portNum == 2)
	{
		while (Length != 0)
		{
			while ( !(UART2TxEmpty & 0x01) );	
			LPC_UART2->THR = *BufferPtr;
			UART2TxEmpty = 0;
			BufferPtr++;
			Length--;
		}
	}
	else if (portNum == 3)
	{
		while (Length != 0)
		{
			while ( !(UART3TxEmpty & 0x01) );	
			LPC_UART3->THR = *BufferPtr;
			UART3TxEmpty = 0;
			BufferPtr++;
			Length--;
		}
	}*/
	return;
}

