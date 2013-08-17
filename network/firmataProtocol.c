
#include <LPC17xx.h>
#include "debug.h"
#include "pwm.h"

#include "network/firmataProtocol.h"
#include "emac.h"
#include "network/MAC_ADDRESSES.h"
//#include "network/arp.h"

#include "ssp.h"
//#include "modules/stepperControl_dSpin.h"

#define FLIP_ENDIAN_16(x) ( (x & 0xff00) >> 8 ) | ( (x & 0x00ff) << 8)

//extern volatile uint16_t stepperPosition;
//extern volatile uint16_t targetPosition;

void parseIncomingFirmataPacket(struct incomingFirmataPacket *ptr) {
	unsigned int numSspBytes;

	// Set the 8-bit output (on/off)
	LPC_GPIO2->FIOPIN = ptr->inputByte;
	debugByte("inputByte: ", ptr->inputByte);

	// Set the PWM outputs (6 channels)
	LPC_PWM1->MR1 = ptr->pwm1;
	LPC_PWM1->MR2 = ptr->pwm2;
	LPC_PWM1->MR3 = ptr->pwm3;
	LPC_PWM1->MR4 = ptr->pwm4;
	LPC_PWM1->MR5 = ptr->pwm5;
	LPC_PWM1->MR6 = ptr->pwm6;
	LPC_PWM1->LER = LER0_EN | LER1_EN | LER2_EN | LER3_EN | LER4_EN | LER5_EN | LER6_EN;


	// SPI:  first byte is numbytes (15 max, 0 means "do nothing")
	numSspBytes = ptr->numSspBytes & 0x0f;
	if (numSspBytes) {
		SSP0Send(ptr->sspCommand, numSspBytes);
	}

/*
	if (ptr->stepperCmd == 'l') {
		stepperControl_goLeft();
	} else if (ptr->stepperCmd == 'r') {
		stepperControl_goRight();
	} else if (ptr->stepperCmd == 's') { // seek to absolute position
		targetPosition = FLIP_ENDIAN_16(ptr->setPosition);
	} else if (ptr->stepperCmd == 'a') { // seek to absolute position
		targetPosition = FLIP_ENDIAN_16(ptr->setPosition);
	}
	if (targetPosition > 800) {
		targetPosition = 800;
	} else if (targetPosition < 20) {
		targetPosition = 20;
	}
*/

}

extern volatile uint8_t triggerLevel;
extern volatile uint8_t triggerDirection;
extern volatile uint8_t triggerChannel;
extern volatile uint8_t triggerEnabled;
extern volatile uint16_t triggerNumSamplesReq;
extern volatile uint8_t adc_weAreSending;

void parseIncomingFirmataControlPacket(struct incomingFirmataControlPacket *ptr) {

	switch(ptr->triggerMode) {
		case TRIGGERMODE_OFF:
			adc_weAreSending = 0;
			triggerEnabled = 0;
			break;
		case TRIGGERMODE_NOW:
			adc_weAreSending = 1;
			break;
		case TRIGGERMODE_RISING:
			triggerDirection = 1;
			triggerEnabled = 1;
			break;
		case TRIGGERMODE_FALLING:
			triggerDirection = 0;
			triggerEnabled = 1;
			break;
		//case TRIGGERMODE_CONTINUOUS:
		//	break;
		default:
			adc_weAreSending = 0;
			triggerEnabled = 0;
			break;
	}

	triggerChannel = ptr->triggerChannel;
	triggerLevel = ptr->triggerLevel;

	triggerNumSamplesReq = 256 * ptr->triggerNumFramesReq; // Number of samples requested, x256
	triggerEnabled = 1;
}


