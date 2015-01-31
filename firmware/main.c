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
#include "network/ip.h"
#include "network/arp.h"
#include "network/DEFAULT_IP_ADDRESS.h"
#include "network/udpCat.h"
#include "network/udpServices/firmataGPO.h"

#include "ssp.h"
#include "adc.h"
#include "dac.h"
#include "pwm.h"

// currentTime is in ms (JavaScript style).  A 32-bit counter
// will roll-over once every 49.7 days
volatile uint32_t currentTime;


void SysTick_Handler (void) {
	// Things to do 100 times per second

	// currentTime is in ms (JavaScript style), but we only update once
	// per 10 ms.
	currentTime += 10;

	// Since we have our own RxFIFO, we have to have our own "timeout" on stale data:
	SSP0_pleaseReceive();
}


int main(void) {
	currentTime = 0;

	// Default IP address, from an include or Makefile:
	char myIpAddress[4] = SELF_IP_ADDR;
	nc_ipaddr_init();
	initArpCache();



	// Enable our blinky LEDs:
	LPC_GPIO1->FIODIR = bit18 | bit20 | bit21 | bit23;

	LPC_GPIO1->FIOSET = bit18;  // Turn on blinky LED #1

	// Initialize serial port for debug messages:
	UARTInit(2, 38400);
	debug("\r\n...init done.");

	setMacAddress();
	setIpAddress(myIpAddress);
	ethernetInitTxBuffers();  // This must occur before ADC init

	LPC_GPIO1->FIOSET = bit20;  // Turn on blinky LED #2

	// Initialize Ethernet:
	Init_EMAC();
	NVIC_EnableIRQ(ENET_IRQn);

	LPC_GPIO1->FIOSET = bit21;  // Turn on blinky LED #3
	debug("Done with Init_EMAC().");

	LPC_GPIO1->FIOSET = bit23;  // Turn on blinky LED #4

	ncDebug("Network online.");

	// ***** BEGIN:  Initialize peripherials

	// Disable UART0 and UART1 for power-saving:
	//LPC_SC->PCONP &= ~((1 << 3) | (1 << 4));

	// Digital outputs for the eFirmata protocol:
	initGPO();

	ADCInit(); // Interrupt usually occurs immediately, so ENET must be initialized already.
	DACInit();
	PWM_Init();
	PWM_Start();
	SSP0Init();

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

