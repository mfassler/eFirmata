
#include <LPC17xx.h>
#include "bitTypes.h"

#include "debug.h"

#include "emac.h"
#include "network/endian.h"
#include "network/ethernet.h"
#include "network/ip.h"
#include "network/udp.h"
#include "network/udpCat.h"

#include "network/udpServices/firmataGPO.h"


// For now, we're only using p0.0, p0.1, p0.6, p0.7, p0.8, p0.9
uint32_t gpo_gpio0_bitmask = bit0 | bit1 | bit6 | bit7 | bit8 | bit9;


void initGPO(void) {
	// Enable outputs for p0.0, p0.1, p0.6, p0.7, p0.8, p0.9
	LPC_GPIO0->FIODIR |= gpo_gpio0_bitmask;
}


void udpGPO(struct ethernetFrame *frame, unsigned int length) {
	const char FIRMATA_ID_SUBTOKEN[3] = "GPO"; // This is GPIO Output
	const uint8_t FIRMATA_GPO_VERSION = 0;

	struct ipPacket *ip;
	struct udpPacket *udp;
	struct gpoCmdOverUdp *cmd;
	unsigned short i;

	(void)length;  // Unused parameter

	ip = (struct ipPacket*) &frame->payload;
	udp = (struct udpPacket*) &ip->data;
	cmd = (struct gpoCmdOverUdp*) &udp->data;

	// Check that this packet starts with the string "eFirmata"
	for (i=0; i<8; i++) {
		if (cmd->idToken[i] != FIRMATA_ID_TOKEN[i]) {
			return;
		}
	}

	// Check that this is for GPO:
	for (i=0; i<3; i++) {
		if (cmd->idSubToken[i] != FIRMATA_ID_SUBTOKEN[i]) {
			return;
		}
	}

	// We only support protocol version 0:
	if (cmd->version != FIRMATA_GPO_VERSION) {
		return;
	}

	LPC_GPIO0->FIOMASK = ~(ntohl(cmd->maskPins[0].mask) & gpo_gpio0_bitmask);
	LPC_GPIO0->FIOPIN = (ntohl(cmd->maskPins[0].pins) & gpo_gpio0_bitmask);
}



