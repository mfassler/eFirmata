

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

	debug("parse ICMP");

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


