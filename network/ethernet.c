
#include <LPC17xx.h>
#include "debug.h"

#include "emac.h"
#include "network/ethernet.h"
#include "network/arp.h"
#include "network/ip.h"
#include "network/firmataProtocol.h"
#include "network/endian.h"


volatile char myMacAddress[6];


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

	debug("init outgoing enet packets");

	for (i = 0; i<NUM_TX_FRAG; i++) {
		oneFrame = (void*) TX_BUF(i);

		for (j=0; j<6; j++) {
			oneFrame->src[j] = myMacAddress[j];
		}

		for (j=0; j < sizeof(((struct ethernetFrame *)0)->payload); j++) {
			oneFrame->payload[j] = 0;
		}
	}

	// Special, for eFirmata_fast:
	bigEtherFrameA->type = htons(0x0800);
	bigEtherFrameB->type = htons(0x0800);
}



#define IAP_LOCATION 0x1fff1ff1;
typedef void (*IAP) (unsigned int [], unsigned int[]);
IAP iap_entry = (IAP) IAP_LOCATION;

void setMacAddress(void) {
	unsigned int command[5];
	unsigned int result[5];

	unsigned int tmp;

	debug("Getting device serial number...");

	// Read device serial number:
	command[0] = 58; // defined on page ~634
	iap_entry(command, result);

	debugLong("1: ", result[1]);
	debugLong("2: ", result[2]);
	debugLong("3: ", result[3]);
	debugLong("4: ", result[4]);

	// ARM is 00:02:f7
	// a local-only address has the bit 02:00:00 set
	// so we'll use 02:02:f7
	myMacAddress[0] = 0x02;
	myMacAddress[1] = 0x02;
	myMacAddress[2] = 0xf7;

	// "hash" the serial number into our MAC address:
	tmp = result[1] ^ result[2] ^ result[3] ^ result[4];

	myMacAddress[3] = (tmp & 0xff0000) >> 16;
	myMacAddress[4] = (tmp & 0xff00) >> 8;
	myMacAddress[5] = tmp & 0xff;

	debugMacAddress("My MAC address: ", (char *) myMacAddress);
}

