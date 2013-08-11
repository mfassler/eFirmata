/****************************************************************************
 *   $Id:: ssp.c 5804 2010-12-04 00:32:12Z usb00423                         $
 *   Project: NXP LPC17xx SSP example
 *
 *   Description:
 *     This file contains SSP code example which include SSP initialization, 
 *     SSP interrupt handler, and APIs for SSP access.
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
#include "LPC17xx.h"			/* LPC17xx Peripheral Registers */
#include "ssp.h"
#include "bitTypes.h"
#include "debug.h"

/* statistics of all the interrupts */
volatile uint32_t interrupt0RxStat = 0;
volatile uint32_t interrupt0OverRunStat = 0;
volatile uint32_t interrupt0RxTimeoutStat = 0;
volatile uint32_t interrupt1RxStat = 0;
volatile uint32_t interrupt1OverRunStat = 0;
volatile uint32_t interrupt1RxTimeoutStat = 0;

void SSP0_IRQHandler(void) {

	uint32_t regValue;

	regValue = LPC_SSP0->MIS; // MIS - Masked Interrupt Status Register

	// RORMIS -- Receive Overrun
	if (regValue & bit0) {
		interrupt0OverRunStat++;
		LPC_SSP0->ICR = bit0;   // Clear the interrupt
	}

	// Read timeout (ie, we should be reading from Rx FIFO...)
	if (regValue & bit1) {
		interrupt0RxTimeoutStat++;
		LPC_SSP0->ICR = bit1;  // Clear the interrupt
	}

	/* please be aware that, in main and ISR, CurrentRxIndex and CurrentTxIndex
	are shared as global variables. It may create some race condition that main
	and ISR manipulate these variables at the same time. SSPSR_BSY checking (polling)
	in both main and ISR could prevent this kind of race condition */

	// RXMIS -- Rx FIFO is at lest half full
	if (regValue & bit2) {
		interrupt0RxStat++;
	}

	// TXMIS -- Tx FIFO is at least half empty
	if (regValue & bit3) {
		// The sendBytes() function will check whether or not there's data to send.
		SSP0_sendBytes();
	}

	return;
}


void SSP1_IRQHandler(void) {

	uint32_t regValue;

	//debug("this is: SSP1_IRQHandler()");
	regValue = LPC_SSP1->MIS;

	// RORMIS -- Receive Overrun
	if (regValue & bit0) {
		interrupt1OverRunStat++;
		LPC_SSP1->ICR = bit0; // Clear the interrupt
	}

	
	// Read timeout (ie, we should be reading from Rx FIFO...)
	if (regValue & bit1) {
		interrupt1RxTimeoutStat++;
		LPC_SSP1->ICR = bit1; // Clear the interrupt
	}

	/* please be aware that, in main and ISR, CurrentRxIndex and CurrentTxIndex
	are shared as global variables. It may create some race condition that main
	and ISR manipulate these variables at the same time. SSPSR_BSY checking (polling)
	in both main and ISR could prevent this kind of race condition */

	// RXMIS -- Rx FIFO is at lest half full
	if (regValue & bit2) {
		interrupt1RxStat++; // Receive until it's empty
	}

	return;
}


void SSP0Init(void) {
	uint8_t nothing=nothing;
	uint8_t i;

	// Enable AHB clock to the SSP0:
	LPC_SC->PCONP |= bit21;

	// The peripheral clock.  Leave at default (divide by 4)
	//setPeripheralClock(PCLK_SSP0, 4);

	// SSP0 is on pins P0.15 through 0.18
	LPC_PINCON->PINSEL0 &= ~(0x3UL<<30); // Clear P0.15
	LPC_PINCON->PINSEL0 |= (0x2UL<<30); // P0.15 is SCK
	LPC_PINCON->PINSEL1 &= ~((0x3<<0)|(0x3<<2)|(0x3<<4));  // Clear P0.16, P0.17, P0.18
	LPC_PINCON->PINSEL1 |= ((0x2<<0)|(0x2<<2)|(0x2<<4)); // P0.16 is SSEL, P0.17 is MISO, P0.18 is MOSI
  
	// ** SSP Control Register **
	// 0x...7 -- Data Size:  8-bit   
	// 0x..0. -- Frame format: SPI, CPOL=0, CPHA=0
	// 0x3f.. -- SCR: 63
	LPC_SSP0->CR0 = 0x3f07;
	//LPC_SSP0->CR0 = 0x3fc7;  // CPOL=1, CPHA=1

	// CPSR, clock prescale register for master mode (not used for slave mode)
	//  Must be 2 or greater.  Must be an even number.
	//  Bit clock = PCLK / (CPSR * (SCR + 1))
	//LPC_SSP0->CPSR = 2;
	LPC_SSP0->CPSR = 8;

	for (i = 0; i < FIFOSIZE; i++) {
		nothing = LPC_SSP0->DR;   // clear the RxFIFO
	}

	// Enable the Interrupt
	NVIC_EnableIRQ(SSP0_IRQn);

	// Enable Interrupts.
	//   bit0: RORIM -- Recieve Overrun
	//   bit1: RTIM  -- Receive Timeout
	LPC_SSP0->IMSC = bit0 | bit1;
	// The TxFIFO interrupt will be set or cleared (elsewhere) as needed.
	//   bit3: TXIM  -- Output FIFO is at least half empty


	// SSP enable (set this after setting up interrupts)
	// ("Master mode" and "no loopback" are left at default)
	LPC_SSP0->CR1 = bit1;

	return;
}


void SSP1Init(void) {

	uint8_t i, Dummy=Dummy;

	// Enable AHB clock to the SSP1.
	LPC_SC->PCONP |= (0x1<<10);

	// The peripheral clock.  Leave at default (divide by 4)
	//setPeripheralClock(PCLK_SSP1, 4);

	// P0.6~0.9 as SSP1
	LPC_PINCON->PINSEL0 &= ~((0x3<<12)|(0x3<<14)|(0x3<<16)|(0x3<<18));
	LPC_PINCON->PINSEL0 |= ((0x2<<12)|(0x2<<14)|(0x2<<16)|(0x2<<18));

	// We use P0.6 as CS for KXP84 #0
	// We use P0.0 as CS for KXP84 #1
	LPC_PINCON->PINSEL0 &= ~(0x3 << 12);  // P0.6 is GPIO
	LPC_PINCON->PINSEL0 &= ~(0x3 << 0);   // P0.0 is GPIO
	LPC_GPIO0->FIODIR |= bit6;  // P0.6 is an output
	LPC_GPIO0->FIODIR |= bit0;  // P0.0 is an output
	LPC_GPIO0->FIOSET = bit6;   // The KXP84 Accelerometer likes it high...
	LPC_GPIO0->FIOSET = bit0;  // The KXP84 Accelerometer likes it high...

	// Set DSS data to 8-bit, Frame format SPI, CPOL = 0, CPHA = 0, and SCR is 15
	LPC_SSP1->CR0 = 0x0707;

	// SSPCPSR clock prescale register, master mode, minimum divisor is 0x02
	LPC_SSP1->CPSR = 0x2;

	for (i = 0; i < FIFOSIZE; i++) {
		Dummy = LPC_SSP1->DR;   // clear the RxFIFO
	}

	// Enable the SSP Interrupt
	NVIC_EnableIRQ(SSP1_IRQn);
	
	// Device select as master, SSP Enabled
	// Master mode
	LPC_SSP1->CR1 = SSPCR1_SSE;
	/* Set SSPINMS registers to enable interrupts */
	/* enable all error related interrupts */
	LPC_SSP1->IMSC = SSPIMSC_RORIM | SSPIMSC_RTIM;
	return;
}



// FIFO ring buffer for SSP0
volatile char txBuffer_SSP0[256];
volatile uint8_t txBuffer_SSP0_produceIdx = 0; // Input
volatile uint8_t txBuffer_SSP0_consumeIdx = 0; // Output

int SSP0Send(char *buf, int length) {
	int i;
	uint8_t remainingSpace;

	debug("this is SSP0Send");
	remainingSpace = txBuffer_SSP0_consumeIdx - txBuffer_SSP0_produceIdx - 1;
	debugByte("remainingSpace: ", remainingSpace);
	debugLong("length: ", length);

	if (remainingSpace < length) {
		return -1;
	}

	for (i=0; i<length; i++) {
		txBuffer_SSP0[txBuffer_SSP0_produceIdx] = *buf;
		txBuffer_SSP0_produceIdx++;
		buf++;
	}

	// If the buffer was originally empty, then we'll need to trigger the send
	//if ((remainingSpace == 255) && i) {
	// doesn't hurt to re-set the interrupt...
	if (i) {
		// Enable the Tx interrupt:
		//   bit3: TXIM  -- Output FIFO is at least half empty
		LPC_SSP0->IMSC |= bit3;
	}

	return length;
}


void SSP0_sendBytes(void) {

	uint8_t numBytesToSend;

	// Check for space in the Tx FIFO:
	while (LPC_SSP0->SR & bit1) {
		numBytesToSend = txBuffer_SSP0_produceIdx - txBuffer_SSP0_consumeIdx;

		if (numBytesToSend) {
			// Add a byte to the TX buffer:
			LPC_SSP0->DR = txBuffer_SSP0[txBuffer_SSP0_consumeIdx];

			// Next byte:
			txBuffer_SSP0_consumeIdx++;

		} else {
			// No more bytes to send, so disable the Tx interrupt:
			LPC_SSP0->IMSC &= ~bit3;
			break;
		}
	}

	return; 
}



void SSP1Send(uint8_t *buf, uint32_t Length) {

	uint32_t i;
	uint8_t Dummy = Dummy;

	debug("SSP1Send()");
	for (i = 0; i < Length; i++) {

		/* Move on only if NOT busy and TX FIFO not full. */
		while ( (LPC_SSP1->SR & (SSPSR_TNF|SSPSR_BSY)) != SSPSR_TNF );
		LPC_SSP1->DR = *buf;
		buf++;
		while ( (LPC_SSP1->SR & (SSPSR_BSY|SSPSR_RNE)) != SSPSR_RNE );
		/* Whenever a byte is written, MISO FIFO counter increments, Clear FIFO 
		on MISO. Otherwise, when SSP0Receive() is called, previous data byte
		is left in the FIFO. */
		Dummy = LPC_SSP1->DR;
	}

	LPC_GPIO0->FIOCLR = bit6;	
}


// This is for use with the KXP84 3-axis Accelerometer chip
void getAccel(uint8_t whichChip, uint16_t *x, uint16_t *y, uint16_t *z) {
	uint8_t nothing=nothing;
	uint8_t LSByte;
	uint8_t MSByte;

	if (whichChip == 0) {
		LPC_GPIO0->FIOCLR = (1 << 6);  // CS low
	} else {
		LPC_GPIO0->FIOCLR = (1 << 0);  // CS low
	}

	// Move on only if NOT busy and TX FIFO not full.
	while ( !(LPC_SSP1->SR & (SSPSR_TNF|SSPSR_BSY)) );

	LPC_SSP1->DR = 0x80;
	while ( !(LPC_SSP1->SR & (SSPSR_BSY|SSPSR_RNE)) );
	nothing = LPC_SSP1->DR;	

	/* TODO: Delay of 200 uS */

	LPC_SSP1->DR = 0xFF;
	while ( !(LPC_SSP1->SR & (SSPSR_BSY|SSPSR_RNE)) );
	MSByte = LPC_SSP1->DR;	

	LPC_SSP1->DR = 0xFF;
	while ( !(LPC_SSP1->SR & (SSPSR_BSY|SSPSR_RNE)) );
	LSByte = LPC_SSP1->DR;	

	*x = (MSByte << 8) | LSByte;

	LPC_SSP1->DR = 0xFF;
	while ( !(LPC_SSP1->SR & (SSPSR_BSY|SSPSR_RNE)) );
	MSByte = LPC_SSP1->DR;	

	LPC_SSP1->DR = 0xFF;
	while ( !(LPC_SSP1->SR & (SSPSR_BSY|SSPSR_RNE)) );
	LSByte = LPC_SSP1->DR;	

	*y = (MSByte << 8) | LSByte;

	LPC_SSP1->DR = 0xFF;
	while ( !(LPC_SSP1->SR & (SSPSR_BSY|SSPSR_RNE)) );
	MSByte = LPC_SSP1->DR;	

	LPC_SSP1->DR = 0xFF;
	while ( !(LPC_SSP1->SR & (SSPSR_BSY|SSPSR_RNE)) );
	LSByte = LPC_SSP1->DR;	

	*z = (MSByte << 8) | LSByte;

	if (whichChip == 0) {
		LPC_GPIO0->FIOSET = bit6;  // CS high
	} else {
		LPC_GPIO0->FIOSET = bit0;  // CS high
	}

	return;
}


void SSPReceive(uint32_t portnum, uint8_t *buf, uint32_t Length) {
	uint32_t i;
 
	for (i = 0; i < Length; i++) {
		// As long as Receive FIFO is not empty, I can always receive. 
		// If it's a loopback test, clock is shared for both TX and RX,
		// no need to write dummy byte to get clock to get the data */
		// if it's a peer-to-peer communication, SSPDR needs to be written
		//b efore a read can take place.
		if (portnum == 0) {
			LPC_SSP0->DR = 0xFF;
			// Wait until the Busy bit is cleared:
			while ( (LPC_SSP0->SR & (SSPSR_BSY|SSPSR_RNE)) != SSPSR_RNE );
			*buf++ = LPC_SSP0->DR;

		} else if ( portnum == 1 ) {

			LPC_SSP1->DR = 0xFF;
			// Wait until the Busy bit is cleared:
			while ( (LPC_SSP1->SR & (SSPSR_BSY|SSPSR_RNE)) != SSPSR_RNE );
			*buf++ = LPC_SSP1->DR;
		}
	}
}


