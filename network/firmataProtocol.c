
#include <LPC17xx.h>
#include "bitTypes.h"
#include "debug.h"
#include "pwm.h"

#include "network/firmataProtocol.h"
#include "emac.h"
#include "network/MAC_ADDRESSES.h"

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


