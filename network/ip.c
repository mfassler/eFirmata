

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


