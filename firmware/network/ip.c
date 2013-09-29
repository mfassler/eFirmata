

#include <type.h>
#include "bitTypes.h"
#include "debug.h"

#include "network/endian.h"
#include "network/ip.h"
#include "network/icmp.h"
#include "network/udp.h"

volatile char myIpAddress_char[4];


// 32-bit big-endian, for quicker operations
// (I can conceive of future protocols where this won't align on 32-bit boundaries.
//  But for now, it always does...):
volatile uint32_t myIpAddress_longBE;


void parseIncomingIpPacket(struct ethernetFrame *frame, unsigned int length) {
	struct ipPacket *pkt;
	pkt = (struct ipPacket *) &frame->payload;

	switch (pkt->protocol) {
		case 1:
			parseIncomingIcmpPacket(frame, length);
			break;
		case 17:
			parseIncomingUdpPacket(frame, length);
			break;
		default:
			debug("unknown protocol");
			break;
	}
}


uint16_t internetChecksum(void * addr, unsigned int count) {
	/*
	 * Compute Internet Checksum for "count" bytes beginning at location "addr".
	 * This is from RFC 1071
	 */

	register long sum = 0;
	uint16_t *ptr;
	ptr = (uint16_t *) addr;

	while (count > 1) {
		//sum += *(uint16_t *) addr++;
		sum += *ptr++;
		count -= 2;
	}

	//  Add left-over byte, if any 
	if (count > 0) {
		//sum += *(uint8_t *) addr;
		sum += *(uint8_t *)ptr;
	}

	// Fold 32-bit sum to 16 bits
	while (sum >> 16) {
		sum = (sum & 0xffff) + (sum >> 16);
	}

	return (uint16_t) ~sum;
}


void setIpAddress(char ipAddr[]) {
	unsigned int i;
	uint32_t tmpIpAddr;

	for (i=0; i<4; i++) {
		myIpAddress_char[i] = ipAddr[i];
	}

	tmpIpAddr = ipAddr[0] << 24;
	tmpIpAddr |= ipAddr[1] << 16;
	tmpIpAddr |= ipAddr[2] << 8;
	tmpIpAddr |= ipAddr[3];

	myIpAddress_longBE = htonl(tmpIpAddr);
	debugLong("IP, BE: ", myIpAddress_longBE);
}

