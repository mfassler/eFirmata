
#include <LPC17xx.h>
#include "debug.h"
#include "pwm.h"

#include "network/firmataProtocol.h"
#include "emac.h"
#include "network/MAC_ADDRESSES.h"

#include "ssp.h"
#include "modules/stepperControl_dSpin.h"

#define FLIP_ENDIAN_16(x) ( (x & 0xff00) >> 8 ) | ( (x & 0x00ff) << 8)

extern volatile uint16_t stepperPosition;
extern volatile uint16_t targetPosition;

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
		case TRIGGERMODE_ONESHOT:
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


// Parse one ethernet frame:
void parseFrame(char* input, unsigned short inputLen) {

	uint16_t ethertype;

	if (inputLen < 22)
		return;

	ethertype = (input[12] << 8) | input[13];

	// Ethertypes are:
	//  0x181b - firmataControl
	//  0x181c - firmata
	//  0x181d - firmataFast

	switch (ethertype) {
		case 0x0800:  // IP
			debug("rx IP");
			break;
		case 0x0806:  // ARP
			debug("rx ARP");
			break;
		case 0x181c:  // eFirmata
			parseIncomingFirmataPacket((struct incomingFirmataPacket *) &input[14]);
			break;
		case 0x181b:  // eFirmataControl
			parseIncomingFirmataControlPacket((struct incomingFirmataControlPacket *) &input[14]);
			break;
		default:
			debugWord("ethertype: ", ethertype);
	}
}


// Outgoing packets:

struct sensorPacket *mySensorPacketA;
struct sensorPacket *mySensorPacketB;
struct bigEtherFrame *bigEtherFrameA;
struct bigEtherFrame *bigEtherFrameB;

void initOutgoingEthernetPackets(void) {
	unsigned int i;

	char myDestAddr[6] = DEST_ADDR;
	char mySrcAddr[6] = SELF_ADDR;
	char prot[2] = EFIRMATA_PROTOCOL;
	char fastProt[2] = EFIRMATA_PROTOCOL_FAST;

	debug("init outgoing enet packets");

	mySensorPacketA = (void*)TX_BUF(0);
	mySensorPacketB = (void*)TX_BUF(1);
	bigEtherFrameA = (void*)TX_BUF(2);
	bigEtherFrameB = (void*)TX_BUF(3);

	for (i=0; i<6; i++) {
		mySensorPacketA->dest[i] = myDestAddr[i];
		mySensorPacketB->dest[i] = myDestAddr[i];
		bigEtherFrameA->dest[i] = myDestAddr[i];
		bigEtherFrameB->dest[i] = myDestAddr[i];
		mySensorPacketA->src[i] = mySrcAddr[i];
		mySensorPacketB->src[i] = mySrcAddr[i];
		bigEtherFrameA->src[i] = mySrcAddr[i];
		bigEtherFrameB->src[i] = mySrcAddr[i];
	}

	for (i=0; i<2; i++) {
		mySensorPacketA->prot[i] = prot[i];
		mySensorPacketB->prot[i] = prot[i];
		bigEtherFrameA->prot[i] = fastProt[i];
		bigEtherFrameB->prot[i] = fastProt[i];
	}

	mySensorPacketA->subProt[0] = ':';
	mySensorPacketA->subProt[1] = '-';
	mySensorPacketA->subProt[2] = ')';
	mySensorPacketA->subProt[3] = 0x01;
	mySensorPacketB->subProt[0] = ':';
	mySensorPacketB->subProt[1] = '-';
	mySensorPacketB->subProt[2] = ')';
	mySensorPacketB->subProt[3] = 0x01;

	for (i=0; i< sizeof( mySensorPacketA->happyMessage); i++) {
		mySensorPacketA->happyMessage[i] = 0x00;
		mySensorPacketB->happyMessage[i] = 0x00;
	}

	mySensorPacketA->fcs = 0x00;
	mySensorPacketB->fcs = 0x00;
	bigEtherFrameA->fcs = 0x00;
	bigEtherFrameB->fcs = 0x00;
}


