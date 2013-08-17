/*
 *  Copyright 2012, Mark Fassler
 *  Licensed under the GPLv3
 *
 */


#include <LPC17xx.h>
#include "bitTypes.h"

#include "uart.h"
#include "debug.h"
#include "timer.h"

#include "emac.h"
#include "network/ethernet.h"

#include "ssp.h"
#include "adc.h"
#include "dac.h"
#include "pwm.h"
//#include "gpioStuff.h"
//#include "modules/quadrature.h"
//#include "modules/stepperControl_dSpin.h"

#include "network/firmataProtocol.h"

// would love for this to be generated from the internal serial number or something... 
#include "network/MAC_ADDRESSES.h"
const char myMacAddress[6] = SELF_ADDR;


volatile uint32_t current_time;

extern volatile uint16_t stepperPosition;
extern volatile uint16_t targetPosition;

void sendOneSensorPacket (void) {
	// Once every 10 ms we send sensor data to the PC.

	// We have two DMA buffers.  We will alternate between those two.
	// (ie, we prepare a packet, then send it, then next time we will
	// prepare the other packet.  So hopefully we wont collide with the
	// EthernetPleaseSend process...)
	struct ethernetFrame *mySensorFrame;
	struct sensorPacket *mySensorPacket;

	// eFirmata-over-ethernet protocol is 0x181c:
	mySensorFrame = ethernetGetNextTxBuffer(0x181c);
	mySensorPacket = (struct sensorPacket *) &(mySensorFrame->payload);
	mySensorPacket->subProt[0] = ':';
	mySensorPacket->subProt[1] = '-';
	mySensorPacket->subProt[2] = ')';
	mySensorPacket->subProt[3] = 0x01;

	// Digital inputs for the eFirmata protocol: P1.24 through P1.31
	mySensorPacket->inputByte = (LPC_GPIO1->FIOPIN >> 24) & 0xff;

	//mySensorPacket->stepperPosition = stepperPosition;
	//mySensorPacket->targetPosition = targetPosition;


	if (LPC_GPIO0->FIOPIN & bit0) {
		mySensorPacket->busyBit = 1;
	} else {
		mySensorPacket->busyBit = 0;
	}

	// Check for data from SSP0:
/*
	i=0;
	while (LPC_SSP0->SR & SSPSR_RNE) {  // RNE = Receive FIFO Not Empty
		// Counting starts at 1
		i++;
		mySensorPacket->happyMessage[i] = LPC_SSP0->DR;
		if (i > 7) {
			break;
		}
	}
	mySensorPacket->happyMessage[0] = i;
*/
	//mySensorPacket->happyMessage[0] = SSP0_readFromFIFO(&mySensorPacket->happyMessage[1], 8);


	//mySensorPacket->quadPositionA = quadrature_getPosition(0);

	ethernetPleaseSend(mySensorFrame, sizeof(struct sensorPacket));
}


void SysTick_Handler (void) {
	// Things to do 100 times per second!
	current_time++;
	sendOneSensorPacket();

	// This is only used for the reflex board
	// This function will check itself before running twice, etc:
	//stepperControl_seekToAbsolutePosition();
}


int main() {
	// Enable our blinky LEDs:
	LPC_GPIO1->FIODIR = bit18 | bit20 | bit21 | bit23;

	LPC_GPIO1->FIOSET = bit18;  // Turn on blinky LED #1

	// Initialize serial port for debug messages:
	UARTInit(2, 38400);
	debug("...init done.");

	LPC_GPIO1->FIOSET = bit20;  // Turn on blinky LED #2

	// Initialize Ethernet:
	Init_EMAC();
	NVIC_EnableIRQ(ENET_IRQn);

	LPC_GPIO1->FIOSET = bit21;  // Turn on blinky LED #3
	debug("Done with Init_EMAC().");

	// Set the src_address, dest_address, etc:
	ethernetInitTxBuffers();  // This must occur before ADC init


	LPC_GPIO1->FIOSET = bit23;  // Turn on blinky LED #4


	// ***** BEGIN:  Initialize peripherials

	// Disable UART0 and UART1 for power-saving:
	//LPC_SC->PCONP &= ~((1 << 3) | (1 << 4));

	// Digital outputs for the eFirmata protocol:
	LPC_GPIO2->FIODIR = 0x00003fc0; // P2.6 through P2.13

	// Digital inputs for the eFirmata protocol:
	//LPC_GPIO1->FIODIR = // P1.24 through P1.31

	ADCInit(); // Interrupt usually occurs immediately, so ENET must be initialized already.
	DACInit();
	PWM_Init();
	PWM_Start();
	//SSP0Init();
	//SSP1Init();

	// Right now, these are only used for the Reflex board:
	//GPIO_Interrupts_Init();

	// ***** END:  Initialize peripherials


	// Start the 100 Hz timer:
	debugLong("SystemCoreClock: ", SystemCoreClock);
	SysTick_Config (SystemCoreClock / 100);

	while (1) {
		delayMs(0, 120);
		LPC_GPIO1->FIOSET = bit20 | bit23;  // heartbeat on

		delayMs(0, 120);
		LPC_GPIO1->FIOCLR = bit20 | bit23;  // heartbeat off
	}
}

