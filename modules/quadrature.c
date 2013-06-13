
#include <LPC17xx.h>
#include "quadrature.h"


/*
	Quadrature Input
		(implemented in software for now, we're not using the built-in
		 quadrature decoders for now)
*/

volatile int16_t quadPositionA;

void EINT3_IRQHandler(void) {
	uint32_t whichInterruptR;
	uint32_t whichInterruptF;

	uint32_t bit21 = 0x00200000; // index enable, active low
	uint32_t bit22 = 0x00400000; // index, falling edge
	uint32_t bit27 = 0x08000000; // quadrature A
	uint32_t bit28 = 0x10000000; // quadratrue B

	whichInterruptR = LPC_GPIOINT->IO0IntStatR;
	whichInterruptF = LPC_GPIOINT->IO0IntStatF;

	// Clear our interrupts, asap:
	//LPC_SC->EXTINT = 0x08; //EINT3; 
	LPC_GPIOINT->IO0IntClr = 0x7fff8fff;

	// Which bit generated the interrupt? (might be multiple at the same time):
	if (whichInterruptF & bit22) {
		if (!(LPC_GPIO0->FIOPIN & bit21)) {
			// ie:  if pin 21 is low (the "enable") and pin 22 falls,
			// then set the position to zero.
			quadPositionA = 0;
		}
	}

	if (whichInterruptF & bit27) {
		if (LPC_GPIO0->FIOPIN & bit28) {
			quadPositionA--;
		} else {
			quadPositionA++;
		}
	}
	if (whichInterruptF & bit28) {
		if (LPC_GPIO0->FIOPIN & bit27) {
			quadPositionA++;
		} else {
			quadPositionA--;
		}
	}
	if (whichInterruptR & bit27) {
		if (LPC_GPIO0->FIOPIN & bit28) {
			quadPositionA++;
		} else {
			quadPositionA--;
		}
	}
	if (whichInterruptR & bit28) {
		if (LPC_GPIO0->FIOPIN & bit27) {
			quadPositionA--;
		} else {
			quadPositionA++;
		}
	}

}

void Quadrature_Init(void) {
	// We will use P0.21, P0.22, P0.27 P0.28  (these are grouped together at the
	// bottom-left corner of the LPCX-presso

	LPC_GPIO0->FIODIR &= ~( (1<<21) | (1<<22) | (1<<27) | (1<<28) );

	// P0.21 is "index enable", active low
	// P0.22 is the index, falling edge
	//   P0.21 and P0.22 are two IR sensors.  We only reset the counter
	//   when moving right, ie, P0.21 is off and P0.22 is falling-edge.
	// P0.27 and P0.28 are the quadrature input

	// Interrupts.  22: falling edge.   27, 28: rising or falling edge

	// Falling-edge interrupts:
	LPC_GPIOINT->IO0IntEnF |= (  (1<<28) | (1<<27) | (1<<22)  );

	// Rising-edge interrupts:
	LPC_GPIOINT->IO0IntEnR |= (  (1<<28) | (1<<27)  );

	NVIC_EnableIRQ(EINT3_IRQn);

	quadrature_updatePosition();
}


void quadrature_updatePosition(void) {

	uint32_t bitMask = (1<<28) | (1<<27) | (1<<22) | (1<<21);

	quadPositionA = (LPC_GPIO0->FIOPIN & bitMask) >> 16;
}

int16_t quadrature_getPosition(uint8_t whichChannel) {
	//quadrature_updatePosition();
	if (whichChannel == 0) {
		return quadPositionA;
	} else {
		return 0x0000;
	}
}

