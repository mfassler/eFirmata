/*
 *  Copyright 2012, Mark Fassler
 *  Licensed under the GPLv3
 *
 */


#include <LPC17xx.h>
#include "uart.h"
#include "debug.h"
#include "emac.h"
#include "ssp.h"
#include "adc.h"
#include "dac.h"
#include "pwm.h"
#include "modules/quadrature.h"
#include "timer.h"

#include "network/firmataProtocol.h"

extern volatile uint16_t ADCValue[ADC_NUM];
extern struct sensorPacket *mySensorPacket;

volatile uint32_t current_time;

void jiffyAction (void)
{
	uint8_t i;

	// Once every 10 ms we send sensor data to the PC.

	// Digital inputs for the eFirmata protocol: P1.24 through P1.31
	mySensorPacket->inputByte = (LPC_GPIO1->FIOPIN >> 24) & 0xff;

	mySensorPacket->adcVal = ADCValue[4];   // bend sensor for right elbow


	getAccel(0, &(mySensorPacket->xAccel0), 
				&(mySensorPacket->yAccel0),
				&(mySensorPacket->zAccel0)  );

	getAccel(1, &(mySensorPacket->xAccel1), 
				&(mySensorPacket->yAccel1),
				&(mySensorPacket->zAccel1)  );


	// Check for data from SSP0:
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


	mySensorPacket->quadPositionA = quadrature_getPosition(0x00);

	ethernetPleaseSend(0, sizeof(struct sensorPacket));
}


void SysTick_Handler (void)
{
	current_time++;
	jiffyAction();
}


int main() {
	// Enable our blinky LEDs:
	LPC_GPIO1->FIODIR = (1 << 18) | (1 << 20) | (1 << 21) | (1 << 23);

	LPC_GPIO1->FIOSET = (1 << 18);  // Turn on blinky LED #1

	// Initialize serial port for debug messages:
	UARTInit(2, 38400);
	debug("...init done.");

	LPC_GPIO1->FIOSET = (1 << 20);  // Turn on blinky LED #2

	// Initialize Ethernet:
	Init_EMAC();
	NVIC_EnableIRQ(ENET_IRQn);

	LPC_GPIO1->FIOSET = (1 << 21);  // Turn on blinky LED #3
	debug("Done with Init_EMAC().");

	LPC_GPIO1->FIOSET = (1 << 23);  // Turn on blinky LED #4


	// ***** BEGIN:  Initialize peripherials

	// Disable UART0 and UART1 for power-saving:
	//LPC_SC->PCONP &= ~((1 << 3) | (1 << 4));

	// Digital outputs for the eFirmata protocol:
	LPC_GPIO2->FIODIR = 0x00003fc0; // P2.6 through P2.13

	// Digital inputs for the eFirmata protocol:
	//LPC_GPIO1->FIODIR = // P1.24 through P1.31

	ADCInit();
	DACInit();
	PWM_Init();
	PWM_Start();
	SSP0Init();
	SSP1Init();
	Quadrature_Init();

	// ***** END:  Initialize peripherials

	// Set the src_address, dest_address, etc:
	initOutgoingEthernetPackets();

	// Start the 100 Hz timer:
	SysTick_Config (SystemCoreClock / 100);

	while (1)
	{
		delayMs(0, 120);
		LPC_GPIO1->FIOSET = (1 << 20) | (1<<23);  // heartbeat

		delayMs(0, 120);
		LPC_GPIO1->FIOCLR = (1 << 20) | (1<<23);  // heartbeat
	}
}

