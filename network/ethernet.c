
#include <LPC17xx.h>
#include "debug.h"

#include "emac.h"
#include "network/MAC_ADDRESSES.h"
#include "network/ethernet.h"
#include "network/arp.h"
#include "network/ip.h"
#include "network/firmataProtocol.h"
#include "network/endian.h"


// Parse one ethernet frame:
void parseFrame(struct ethernetFrame *input, unsigned int inputLen) {

	uint16_t ethertype;

	if (inputLen < 22)
		return;

	ethertype = ntohs(input->type);

	// Ethertypes are:
	//  0x181b - firmataControl
	//  0x181c - firmata
	//  0x181d - firmataFast

	switch (ethertype) {
		case 0x0800:  // IP
			parseIncomingIpPacket(input, inputLen);
			break;
		case 0x0806:  // ARP
			parseIncomingArpPacket((struct arpPacket *) &input->payload);
			break;
		case 0x181c:  // eFirmata
			parseIncomingFirmataPacket((struct incomingFirmataPacket *) &input->payload);
			break;
		default:
			debugWord("unknown ethertype: ", ethertype);
	}
}


// Outgoing packets:

// Four buffers for outgoing data:
//struct ethernetFrame *outFrameA = (void*) TX_BUF(0);
//struct ethernetFrame *outFrameB = (void*) TX_BUF(1);
//struct ethernetFrame *outFrameC = (void*) TX_BUF(2);
//struct ethernetFrame *outFrameD = (void*) TX_BUF(3);

// These two are reserved for fast ADC data:
struct ethernetFrame *bigEtherFrameA = (void*) TX_BUF(4);
struct ethernetFrame *bigEtherFrameB = (void*) TX_BUF(5);


struct ethernetFrame *ethernetGetNextTxBuffer(uint16_t ethertype) {
	static unsigned int whichBuffer = 0;

	struct ethernetFrame *aFrame;
	aFrame = (void*) TX_BUF(whichBuffer);

	aFrame->type = htons(ethertype);

	whichBuffer++;
	if (whichBuffer > 3) {
		whichBuffer = 0;
	}

	return aFrame;
}


void ethernetInitTxBuffers(void) {
	unsigned int i, j;

	struct ethernetFrame *oneFrame;

	char myDestAddr[6] = DEST_ADDR;
	char mySrcAddr[6] = SELF_ADDR;

	debug("init outgoing enet packets");

	for (i = 0; i<NUM_TX_FRAG; i++) {
		oneFrame = (void*) TX_BUF(i);

		for (j=0; j<6; j++) {
			oneFrame->dest[j] = myDestAddr[j];
			oneFrame->src[j] = mySrcAddr[j];
		}

		for (j=0; j < sizeof(((struct ethernetFrame *)0)->payload); j++) {
			oneFrame->payload[j] = 0;
		}

		oneFrame->fcs = 0;
	}

	// Special, for eFirmata_fast:
	//bigEtherFrameA->type = htons(EFIRMATA_PROTOCOL_FAST);
	//bigEtherFrameB->type = htons(EFIRMATA_PROTOCOL_FAST);
	bigEtherFrameA->type = htons(0x0800);
	bigEtherFrameB->type = htons(0x0800);
}


