
#include <LPC17xx.h>
#include "debug.h"
#include "pwm.h"

#include "firmataProtocol.h"
#include "emac.h"
#include "MAC_ADDRESSES.h"

#include "ssp.h"

void parseFrame(char* input, unsigned short inputLen)
{
	unsigned short protocol;
	unsigned int numSspBytes;
	unsigned int i=0;

	protocol = (input[12] << 8) | input[13];

	debugWord("protocol: ", protocol);

	if (inputLen < 22)
		return;

	if ((input[12] == 0x18) && (input[13] == 0x1c))  // etherType == eFirmata
	{
		LPC_GPIO2->FIOPIN = (input[14] << 6);
		// byte#15..
		// byte#16..
		LPC_PWM1->MR1 = input[17];
		LPC_PWM1->MR2 = input[18];
		LPC_PWM1->MR3 = input[19];
		LPC_PWM1->MR4 = input[20];
		LPC_PWM1->MR5 = input[21];
		LPC_PWM1->MR6 = input[22];
		LPC_PWM1->LER = LER0_EN | LER1_EN | LER2_EN | LER3_EN | LER4_EN | LER5_EN | LER6_EN;
		// SPI:  first byte is numbytes (15 max, 0 means "do nothing")
		numSspBytes = input[23] & 0x0f;
		for (i=0; i<numSspBytes; i++) {
			SSP0Send(&input[24+i], 1);
		}
	}
}


// Outgoing packets:

struct sensorPacket *mySensorPacket;
struct bigEtherFrame *bigEtherFrameA;
struct bigEtherFrame *bigEtherFrameB;

void initOutgoingEthernetPackets(void)
{
	unsigned int i;

	char myDestAddr[6] = DEST_ADDR;
	char mySrcAddr[6] = SELF_ADDR;
	char prot[2] = EFIRMATA_PROTOCOL;
	char fastProt[2] = EFIRMATA_PROTOCOL_FAST;

	mySensorPacket = (void*)TX_BUF(0);
	bigEtherFrameA = (void*)TX_BUF(1);
	bigEtherFrameB = (void*)TX_BUF(2);

	for (i=0; i<6; i++)
	{
		mySensorPacket->dest[i] = myDestAddr[i];
		bigEtherFrameA->dest[i] = myDestAddr[i];
		bigEtherFrameB->dest[i] = myDestAddr[i];
		mySensorPacket->src[i] = mySrcAddr[i];
		bigEtherFrameA->src[i] = mySrcAddr[i];
		bigEtherFrameB->src[i] = mySrcAddr[i];
	}
	for (i=0; i<2; i++)
	{
		mySensorPacket->prot[i] = prot[i];
		bigEtherFrameA->prot[i] = fastProt[i];
		bigEtherFrameB->prot[i] = fastProt[i];
	}
	mySensorPacket->subProt[0] = ':';
	mySensorPacket->subProt[1] = '-';
	mySensorPacket->subProt[2] = ')';
	mySensorPacket->subProt[3] = 0x01;
	mySensorPacket->nothing = 0x69;

	for (i=0; i< sizeof( mySensorPacket->happyMessage); i++)
	{
		mySensorPacket->happyMessage[i] = 0x00;
	}
	mySensorPacket->fcs = 0x00;
	bigEtherFrameA->fcs = 0x00;
	bigEtherFrameB->fcs = 0x00;
}


