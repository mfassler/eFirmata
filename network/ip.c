

#include <type.h>
#include "bitTypes.h"
#include "debug.h"

#include "network/ip.h"
#include "network/icmp.h"


void parseIncomingIpPacket(struct ethernetFrame *frame, unsigned int length) {
	struct ipPacket *pkt;
	pkt = (struct ipPacket *) &frame->payload;

	switch (pkt->protocol) {
		case 1:
			parseIncomingIcmpPacket(frame, length);
			break;
		default:
			debug("unknown protocol");
			break;
	}
}


