
#include <LPC17xx.h>
#include <type.h>
#include "bitTypes.h"

#include "debug.h"

#include "pwm.h"


void PWM_Init(void) {

	// We want to use all 6 PWMs.  Set the GPIOs for PWM:
	LPC_PINCON->PINSEL4 &= ~0x00000fff;
	LPC_PINCON->PINSEL4 |= 0x00000555;

	// Counter reset:
	LPC_PWM1->TCR = bit1;

	// *** Set the clock for PWMs:
	//  (TODO:  pclk should be checked against pclkdiv)
	// By default, pclk is 1/4 of system clock, so 25 MHz.
	// PWM Freq =  (pclk / (PR +1)) / MR0
	LPC_PWM1->PR = 3;  //  ~25 KHz
	//LPC_PWM1->PR = 240;  //  ~400 Hz

	// We will use a clock of 256 steps, which allows us to set the
	// the widths of the 6 PWMs as 8-bit unsigned ints (0==0%, 255==100%)
	LPC_PWM1->MR0 = 0xff;  // PWM_cycle 

	// Turn them all off:
	LPC_PWM1->MR1 = 0;
	LPC_PWM1->MR2 = 0;
	LPC_PWM1->MR3 = 0;
	LPC_PWM1->MR4 = 0;
	LPC_PWM1->MR5 = 0;
	LPC_PWM1->MR6 = 0;

	// LER, Latch Enable Register:  Put our MRn settings into action:
	LPC_PWM1->LER = bit0 | bit1 | bit2 | bit3 | bit4 | bit5 | bit6;
}


void PWM_Start(void) {

	// Enable all 6 PWMs (all single-edge):
	LPC_PWM1->PCR = bit9 | bit10 | bit11 | bit12 | bit13 | bit14;

	// Counter enable, PWM enable:
	LPC_PWM1->TCR = bit0 | bit3;
}


void PWM_Stop(void) {

	LPC_PWM1->PCR = 0;
	LPC_PWM1->TCR = 0;
}

