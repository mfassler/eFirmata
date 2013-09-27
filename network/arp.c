
#include <type.h>
#include "bitTypes.h"
#include "debug.h"

#include "emac.h"
#include "network/ethernet.h"
#include "network/arp.h"

extern char myMacAddress[];
extern volatile char myIpAddress_char[];

void parseIncomingArpPacket(struct arpPacket *pkt) {

	struct ethernetFrame *aFrame;
	struct arpPacket *replyPkt;

	// oper==1 is a "request", but we have to flip to network-endian
	if ((pkt->oper == 0x0100) &&
		(pkt->targetIpAddress[0] == myIpAddress_char[0]) &&
		(pkt->targetIpAddress[1] == myIpAddress_char[1]) &&
		(pkt->targetIpAddress[2] == myIpAddress_char[2]) &&
		(pkt->targetIpAddress[3] == myIpAddress_char[3]) ) {

		debug("ARP: They're looking for me!");

		aFrame = ethernetGetNextTxBuffer(0x0806);
		replyPkt = (struct arpPacket *) &(aFrame->payload);

		aFrame->dest[0] = pkt->senderMacAddress[0];
		aFrame->dest[1] = pkt->senderMacAddress[1];
		aFrame->dest[2] = pkt->senderMacAddress[2];
		aFrame->dest[3] = pkt->senderMacAddress[3];
		aFrame->dest[4] = pkt->senderMacAddress[4];
		aFrame->dest[5] = pkt->senderMacAddress[5];

		// The ARP packet:
		replyPkt->htype = 0x0100;  // Ethernet (flipped for network-endian)
		replyPkt->ptype = 0x0008; // IP (flipped for network-endian)
		replyPkt->hlen = 6; // Ethernet addresses are 6 octets
		replyPkt->plen = 4; // IPv4 addresses are 4 octets
		replyPkt->oper = 0x0200; // This is a reply (flipped for network-endian)

		// Me:
		replyPkt->senderMacAddress[0] = myMacAddress[0];
		replyPkt->senderMacAddress[1] = myMacAddress[1];
		replyPkt->senderMacAddress[2] = myMacAddress[2];
		replyPkt->senderMacAddress[3] = myMacAddress[3];
		replyPkt->senderMacAddress[4] = myMacAddress[4];
		replyPkt->senderMacAddress[5] = myMacAddress[5];
		replyPkt->senderIpAddress[0] = myIpAddress_char[0];
		replyPkt->senderIpAddress[1] = myIpAddress_char[1];
		replyPkt->senderIpAddress[2] = myIpAddress_char[2];
		replyPkt->senderIpAddress[3] = myIpAddress_char[3];

		// The other guy:
		replyPkt->targetMacAddress[0] = pkt->senderMacAddress[0];
		replyPkt->targetMacAddress[1] = pkt->senderMacAddress[1];
		replyPkt->targetMacAddress[2] = pkt->senderMacAddress[2];
		replyPkt->targetMacAddress[3] = pkt->senderMacAddress[3];
		replyPkt->targetMacAddress[4] = pkt->senderMacAddress[4];
		replyPkt->targetMacAddress[5] = pkt->senderMacAddress[5];
		replyPkt->targetIpAddress[0] = pkt->senderIpAddress[0];
		replyPkt->targetIpAddress[1] = pkt->senderIpAddress[1];
		replyPkt->targetIpAddress[2] = pkt->senderIpAddress[2];
		replyPkt->targetIpAddress[3] = pkt->senderIpAddress[3];

		ethernetPleaseSend(aFrame, SIZE_OF_ARP_IN_ETHERNET);
	}
}


