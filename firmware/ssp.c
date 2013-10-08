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

#include <LPC17xx.h>
#include "bitTypes.h"

#include "debug.h"
#include "network/udpCat.h"

#include "ssp.h"

#include "network/udpServices/firmataSPI.h"

// increments ~100 times per second  (=> rolls over every ~497 days)
extern volatile uint32_t currentTime; 


/* statistics of all the interrupts */
volatile uint32_t interrupt0OverRunStat = 0;
//volatile uint32_t interrupt1OverRunStat = 0;



/**
 * The SSP hardware has its own FIFO buffers:  
 *   an 8 frame input FIFO buffer and
 *   an 8 frame output FIFO buffer.
 *    (one frame can be 4 to 16 bits)
 * 
 * We will make our own 256 frame FIFO buffers.  
 *
 * TX means:  PC -> eFirmata -> MOSI (master out slave in)
 * RX means:  PC <- eFirmata <- MISO (master in slave out)
 *
 * A "produce" index points to data going *into* the FIFO
 * A "consume" index points to data being read *out of* the FIFO
 *
 */


// FIFO ring buffers for SSP0
volatile uint16_t SSP0_txBuffer[256];
volatile uint8_t SSP0_txBuffer_produceIdx = 0; // Input (from software)
volatile uint8_t SSP0_txBuffer_consumeIdx = 0; // Output (to SSP)

volatile uint16_t SSP0_rxBuffer[256];
volatile uint8_t SSP0_rxBuffer_produceIdx = 0; // Input (from SSP)
volatile uint8_t SSP0_rxBuffer_consumeIdx = 0; // Output (to software)


/**
 * Read RX data out of the hardware 8-FIFO and into the software 256-FIFO:
 *  MISO
 */
void SSP0_rxBufferMove(void) {
	while (LPC_SSP0->SR & bit2) { // RxFIFO not empty
		SSP0_rxBuffer[SSP0_rxBuffer_produceIdx] = LPC_SSP0->DR;
		SSP0_rxBuffer_produceIdx++;
	}
}


void SSP0_pleaseSend(char *buf, uint8_t bufLen) {
	uint8_t i;
	uint8_t remainingSpace;

	for (i=0; i<bufLen; i++) {
		remainingSpace = SSP0_txBuffer_consumeIdx - SSP0_txBuffer_produceIdx - 1;
		if (remainingSpace) {
			SSP0_txBuffer[SSP0_txBuffer_produceIdx] = buf[i];
			SSP0_txBuffer_produceIdx++;
		}
	}

	SSP0_tryToSend();
}


void SSP0_pleaseReceive(void) {

	//static uint32_t prevTime = 0;

	uint8_t framesToSend;

	framesToSend = SSP0_rxBuffer_produceIdx - SSP0_rxBuffer_consumeIdx;

	if (framesToSend == 0) {
		return;
	}

/*
	// We only send 100 times per second, max:
	if ((currentTime - prevTime) < 1) {
		return;
	}
*/

	udpSPI_replyToSender(SSP0_rxBuffer, SSP0_rxBuffer_consumeIdx, framesToSend);

	SSP0_rxBuffer_consumeIdx += framesToSend;
	//prevTime = currentTime;
}


void SSP0_IRQHandler(void) {
	uint32_t regValue;

	// MIS - Masked Interrupt Status Register
	regValue = LPC_SSP0->MIS;

	// RORMIS -- Receive Overrun
	if (regValue & bit0) {
		interrupt0OverRunStat++;
		LPC_SSP0->ICR = bit0;   // Clear the interrupt
		ncDebug(" *** SSP0: Recieve Overrun ");
	}

	// Read timeout (ie, we should be reading from Rx FIFO...)
	if (regValue & bit1) {
		LPC_SSP0->ICR = bit1;  // Clear the interrupt

		// Copy data into bigger buffer:
		SSP0_rxBufferMove();

		// Maybe we'll send a UDP packet back to the workstation:
		SSP0_pleaseReceive();
	}

	// RXMIS -- Rx FIFO is at lest half full
	if (regValue & bit2) {
		// there is no interrupt to clear here, apparently

		// Copy data into bigger buffer:
		SSP0_rxBufferMove();

		// Maybe we'll send a UDP packet back to the workstation:
		SSP0_pleaseReceive();
	}

	// TXMIS -- Tx FIFO is at least half empty
	if (regValue & bit3) {
		// there is no interrupt to clear here, apparently
		SSP0_tryToSend();
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
	LPC_PINCON->PINSEL0 &= ~(0x3UL << 30);  // Clear P0.15
	LPC_PINCON->PINSEL0 |=  (0x2UL << 30);  // P0.15 is SCK
	LPC_PINCON->PINSEL1 &= ~((0x3 << 0) | (0x3 << 2) | (0x3 << 4));  // Clear P0.16, P0.17, P0.18
	LPC_PINCON->PINSEL1 |=  ((0x2 << 0) | (0x2 << 2) | (0x2 << 4));  // P0.16 is SSEL, P0.17 is MISO, P0.18 is MOSI

	// Manual control of SSEL:
	//LPC_PINCON->PINSEL1 |= ((0x2<<2)|(0x2<<4)); // P0.16 is SSEL, P0.17 is MISO, P0.18 is MOSI
	//LPC_GPIO0->FIODIR |= bit16; // P0.16 is an output
	//LPC_GPIO0->FIOSET = bit16; // P0.16 defaults to high.
  
	// ** SSP Control Register **
	// 0x...7 -- Data Size:  8-bit   
	// 0x..0. -- Frame format: SPI, CPOL=0, CPHA=0
	// 0x3f.. -- SCR: 63
	//LPC_SSP0->CR0 = 0x3f07;
	LPC_SSP0->CR0 = 0x3fc7;  // CPOL=1, CPHA=1

	// CPSR, clock prescale register for master mode (not used for slave mode)
	//  Must be 2 or greater.  Must be an even number.
	//  Bit clock = PCLK / (CPSR * (SCR + 1))
	//LPC_SSP0->CPSR = 2;
	//LPC_SSP0->CPSR = 250;
	LPC_SSP0->CPSR = 4;

	// Clear the RxFIFO:
	for (i = 0; i < FIFOSIZE; i++) {
		nothing = LPC_SSP0->DR;
	}

	// Enable the Interrupt
	NVIC_EnableIRQ(SSP0_IRQn);

	// Enable Interrupts.
	//   bit0: RORIM -- Recieve Overrun
	//   bit1: RTIM  -- Receive Timeout
	//   bit2: RXIM  -- RxFIFO is at least half full
	LPC_SSP0->IMSC = bit0 | bit1 | bit2;
	// The TxFIFO interrupt will be set or cleared (elsewhere) as needed.
	//   bit3: TXIM  -- Output FIFO is at least half empty


	// SSP enable (set this after setting up interrupts)
	// ("Master mode" and "no loopback" are left at default)
	LPC_SSP0->CR1 = bit1;

	return;
}


/**
 * This is for MOSI:  Master Out, Slave In
 */
void SSP0_tryToSend(void) {
	uint8_t numBytesToRead;

	numBytesToRead = SSP0_txBuffer_produceIdx - SSP0_txBuffer_consumeIdx;

	if (numBytesToRead) {
		// Turn on the Tx Interrupt:
		LPC_SSP0->IMSC = bit0 | bit1 | bit2 | bit3;
	} else {
		// Turn off the Tx Interrupt:
		LPC_SSP0->IMSC = bit0 | bit1 | bit2;
	}

	while (numBytesToRead) {

		if (LPC_SSP0->SR & bit1) {  // TxFIFO not full.  So feed in a byte:

			// Take a byte from our 256-byte SW buffer and add it to the 8-frame HW buffer
			// (which will automatically trigger the HW to start sending):
			LPC_SSP0->DR = SSP0_txBuffer[SSP0_txBuffer_consumeIdx];
			SSP0_txBuffer_consumeIdx++;

			numBytesToRead = SSP0_txBuffer_produceIdx - SSP0_txBuffer_consumeIdx;

			// Check for rx data:
			if (LPC_SSP0->SR & bit2) {
				SSP0_rxBuffer[SSP0_rxBuffer_produceIdx] = LPC_SSP0->DR;
				SSP0_rxBuffer_produceIdx++;
			}
		} else {
			//ncDebug("tx full?");
			break;
		}
	}
}



