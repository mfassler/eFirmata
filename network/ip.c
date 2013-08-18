

#include <type.h>
#include "bitTypes.h"
#include "debug.h"

#include "network/ip.h"
#include "network/icmp.h"
#include "network/udp.h"


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


