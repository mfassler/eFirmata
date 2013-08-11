
#include <LPC17xx.h>
#include <type.h>

#include "pwm.h"
#include "debug.h"


void PWM_Init(void) {

	LPC_PINCON->PINSEL4 = 0x00000555;	// set GPIOs for all PWM pins on PWM

	LPC_PWM1->TCR = TCR_RESET;	// Counter Reset
	//LPC_PWM1->PR = 0x00;		// count frequency:Fpclk
	// TODO:  pclk should be checked against pclkdiv:
	// PWM Freq =  (pclk / (PR +1)) / MR0
	LPC_PWM1->PR = 3;  // count frequency / PWM_cycle = ~25 KHz
	//LPC_PWM1->PR = 240;  // count frequency / PWM_cycle = ~400 Hz
	LPC_PWM1->MCR = PWMMR0I;  // interrupt on PWMMR0, reset on PWMMR0, reset TC if PWM matches
	LPC_PWM1->MR0 = 0xff;  // count frequency / PWM_cycle = ~25 KHz
	LPC_PWM1->MR1 = 0;
	LPC_PWM1->MR2 = 0;
	LPC_PWM1->MR3 = 0;
	LPC_PWM1->MR4 = 0;
	LPC_PWM1->MR5 = 0;
	LPC_PWM1->MR6 = 0;

	// all PWM latch enabled
	LPC_PWM1->LER = LER0_EN | LER1_EN | LER2_EN | LER3_EN | LER4_EN | LER5_EN | LER6_EN;
}


void PWM_Set(uint32_t cycle, uint32_t offset ) {

	LPC_PWM1->MR0 = cycle;
	LPC_PWM1->MR1 = cycle * 5/6 + offset;
	LPC_PWM1->MR2 = cycle * 2/3 + offset;
	LPC_PWM1->MR3 = cycle * 1/2 + offset;
	LPC_PWM1->MR4 = cycle * 1/3 + offset;
	LPC_PWM1->MR5 = cycle * 1/6 + offset;
	LPC_PWM1->MR6 = offset;

	/* The LER will be cleared when the Match 0 takes place, in order to
	load and execute the new value of match registers, all the PWMLERs need to
	reloaded. all PWM latch enabled */
	LPC_PWM1->LER = LER0_EN | LER1_EN | LER2_EN | LER3_EN | LER4_EN | LER5_EN | LER6_EN;
}


void PWM_Start(void) {

	// All single edge, all enable
	LPC_PWM1->PCR = PWMENA1 | PWMENA2 | PWMENA3 | PWMENA4 | PWMENA5 | PWMENA6;
	LPC_PWM1->TCR = TCR_CNT_EN | TCR_PWM_EN; // counter enable, PWM enable
}


void PWM_Stop(void) {
	LPC_PWM1->PCR = 0;
	LPC_PWM1->TCR = 0x00;
}

