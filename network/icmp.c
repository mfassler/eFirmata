

#include <type.h>
#include "bitTypes.h"
#include "debug.h"

//#include <string.h>

#include "network/ip.h"
#include "network/icmp.h"
#include "emac.h"


void parseIncomingIcmpPacket(struct ethernetFrame *frame, unsigned int length) {
	struct ipPacket *ip;
	struct icmpPacket *icmp;

	ip = (struct ipPacket *) &frame->payload;
	icmp = (struct icmpPacket *) &ip->data;

	switch (icmp->type) {
		case 8: // Echo request
			icmp_answerPing(frame, length);
			break;
		default:
			debugByte("Unknown ICMP packet type: ", icmp->type);
			break;
	}
}


void icmp_answerPing(struct ethernetFrame *request, unsigned int length) {
	struct ethernetFrame *reply;
	struct ipPacket *requestIp;
	struct ipPacket *replyIp;
	struct icmpPacket *replyIcmp;
	unsigned short ipLen, icmpLen;
	unsigned short i;

	reply = ethernetGetNextTxBuffer(0x0800);

	requestIp = (struct ipPacket *) &request->payload;
	replyIp = (struct ipPacket *) &reply->payload;
	replyIcmp = (struct icmpPacket *) &replyIp->data;

	ipLen = length - ((unsigned long)replyIp - (unsigned long)reply);
	icmpLen = length - ((unsigned long)replyIcmp - (unsigned long)reply);

	// First, make an exact copy of the entire IP packet:
	//memcpy(replyIp, requestIp, (length-14));  // <-- this no worky...  :-(
	for (i=0; i < ipLen; i++) {
		((char *) replyIp)[i] = ((char *) requestIp)[i];
	}

	// Set the receiver (ethernet frame)  (sender should already be set)
	reply->dest[0] = request->src[0];
	reply->dest[1] = request->src[1];
	reply->dest[2] = request->src[2];
	reply->dest[3] = request->src[3];
	reply->dest[4] = request->src[4];
	reply->dest[5] = request->src[5];

	replyIp->identification = 0;
	//replyIp->flagsAndFragOffset = 0;

	// Swap sender and receiver (IP packet):
	replyIp->srcIpAddr = requestIp->destIpAddr;
	replyIp->destIpAddr = requestIp->srcIpAddr;

	replyIcmp->type = 0; // Echo reply

	replyIcmp->checksum = 0;
	replyIcmp->checksum = internetChecksum(replyIcmp, icmpLen);

	//  This will already be correct, since we copied the header from the sender...
	//replyIp->headerChecksum = 0;
	//replyIp->headerChecksum = internetChecksum(replyIp, 20);

	ethernetPleaseSend(reply, length);
}

