
#include <type.h>
#include "bitTypes.h"
#include "debug.h"

#include "emac.h"
#include "network/endian.h"
#include "network/ip.h"
#include "network/ethernet.h"
#include "network/arp.h"


// I'm too lazy to make a proper hashmap right now (I'm only
// caching one, single IP address at them moment...)
struct arpCacheEntry arpCache[4];
extern uint32_t currentTime;  // in ms, rolls over once per 49 days
#define STALE_TIME 180000 // 3 minutes in milliseconds
//#define STALE_TIME 5000 // 5 seconds in milliseconds

void initArpCache() {
	int i;

	for (i =0; i<4; i++) {
		arpCache[i].ipAddrBE = 0;
		arpCache[i].timestamp = 0;
		arpCache[i].macAddr[0] = 0;
		arpCache[i].macAddr[1] = 0;
		arpCache[i].macAddr[2] = 0;
		arpCache[i].macAddr[3] = 0;
		arpCache[i].macAddr[4] = 0;
		arpCache[i].macAddr[5] = 0;
		arpCache[i].status = 0;
	}
}



/**
 * Someone is asking for us for our MAC address.
 */
void rxArpRequest(struct arpPacket *pkt) {
	struct ethernetFrame *aFrame;
	struct arpPacket *replyPkt;

	if ((pkt->targetIpAddress[0] == myIpAddress_char[0]) &&
		(pkt->targetIpAddress[1] == myIpAddress_char[1]) &&
		(pkt->targetIpAddress[2] == myIpAddress_char[2]) &&
		(pkt->targetIpAddress[3] == myIpAddress_char[3])) {

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


/**
 * Someone is answering our question
 */
void rxArpReply(struct arpPacket *pkt) {

	int i=0;

	uint32_t sender_ipAddrBE;

	if ((pkt->targetIpAddress[0] == myIpAddress_char[0]) &&
		(pkt->targetIpAddress[1] == myIpAddress_char[1]) &&
		(pkt->targetIpAddress[2] == myIpAddress_char[2]) &&
		(pkt->targetIpAddress[3] == myIpAddress_char[3])) {

		//debug("rxArpReply");

		sender_ipAddrBE = (pkt->senderIpAddress[0]) |
		                  (pkt->senderIpAddress[1] << 8) |
		                  (pkt->senderIpAddress[2] << 16) |
		                  (pkt->senderIpAddress[3] << 24);

		// Check "did we actually ask for this IP address?":
		if ((sender_ipAddrBE == arpCache[i].ipAddrBE) && 
		    (arpCache[i].status != 0)) {

			arpCache[i].timestamp = currentTime;
			arpCache[i].macAddr[0] = pkt->senderMacAddress[0];
			arpCache[i].macAddr[1] = pkt->senderMacAddress[1];
			arpCache[i].macAddr[2] = pkt->senderMacAddress[2];
			arpCache[i].macAddr[3] = pkt->senderMacAddress[3];
			arpCache[i].macAddr[4] = pkt->senderMacAddress[4];
			arpCache[i].macAddr[5] = pkt->senderMacAddress[5];
			arpCache[i].status = 2; // 2== replyReceived
		}
	}
}



/**
 * We are asking someone else for their MAC address
 */
void txArpRequest(uint32_t ipAddrBE) {
	struct ethernetFrame *aFrame;
	struct arpPacket *requestPkt;

	aFrame = ethernetGetNextTxBuffer(0x0806);
	requestPkt = (struct arpPacket *) &(aFrame->payload);

	// Broadcast our question to the entire LAN:
	aFrame->dest[0] = 0xff;
	aFrame->dest[1] = 0xff;
	aFrame->dest[2] = 0xff;
	aFrame->dest[3] = 0xff;
	aFrame->dest[4] = 0xff;
	aFrame->dest[5] = 0xff;

	// The ARP packet:
	requestPkt->htype = 0x0100;  // Ethernet (flipped for network-endian)
	requestPkt->ptype = 0x0008; // IP (flipped for network-endian)
	requestPkt->hlen = 6; // Ethernet addresses are 6 octets
	requestPkt->plen = 4; // IPv4 addresses are 4 octets
	requestPkt->oper = 0x0100; // This is a request (flipped for network-endian)

	// Me:
	requestPkt->senderMacAddress[0] = myMacAddress[0];
	requestPkt->senderMacAddress[1] = myMacAddress[1];
	requestPkt->senderMacAddress[2] = myMacAddress[2];
	requestPkt->senderMacAddress[3] = myMacAddress[3];
	requestPkt->senderMacAddress[4] = myMacAddress[4];
	requestPkt->senderMacAddress[5] = myMacAddress[5];
	requestPkt->senderIpAddress[0] = myIpAddress_char[0];
	requestPkt->senderIpAddress[1] = myIpAddress_char[1];
	requestPkt->senderIpAddress[2] = myIpAddress_char[2];
	requestPkt->senderIpAddress[3] = myIpAddress_char[3];

	// The other guy:
	requestPkt->targetMacAddress[0] = 0x00;
	requestPkt->targetMacAddress[1] = 0x00;
	requestPkt->targetMacAddress[2] = 0x00;
	requestPkt->targetMacAddress[3] = 0x00;
	requestPkt->targetMacAddress[4] = 0x00;
	requestPkt->targetMacAddress[5] = 0x00;
	// TODO - can this be typcast in a single step?
	requestPkt->targetIpAddress[0] = (ipAddrBE & 0x000000ff);
	requestPkt->targetIpAddress[1] = (ipAddrBE & 0x0000ff00) >> 8;
	requestPkt->targetIpAddress[2] = (ipAddrBE & 0x00ff0000) >> 16;
	requestPkt->targetIpAddress[3] = (ipAddrBE & 0xff000000) >> 24;

	ethernetPleaseSend(aFrame, SIZE_OF_ARP_IN_ETHERNET);
}



/**
 * This is how an application tries to find a MAC address
 */
int arpCacheLookup(uint32_t ipAddrBE, char* macAddr) {

	int i=0;
	// We're not doing a proper hashmap yet.  Just check the
	// first entry
	if ((ipAddrBE == arpCache[i].ipAddrBE) &&
	    (arpCache[i].status == 2)) {
		//debug("arpCacheLookup, hit");

		if ((currentTime - arpCache[i].timestamp) > STALE_TIME) {
			// Refresh the cache, but keep using it
			//debug("arpCacheLookup, refresh");
			txArpRequest(ipAddrBE);
		}

		macAddr[0] = arpCache[i].macAddr[0];
		macAddr[1] = arpCache[i].macAddr[1];
		macAddr[2] = arpCache[i].macAddr[2];
		macAddr[3] = arpCache[i].macAddr[3];
		macAddr[4] = arpCache[i].macAddr[4];
		macAddr[5] = arpCache[i].macAddr[5];

		return 0; // cache hit

	} else {
		//debug("arpCacheLookup, miss");

		arpCache[i].ipAddrBE = ipAddrBE;
		arpCache[i].status = 1; // 1== requestSent

		txArpRequest(ipAddrBE);
		return -1; // cache miss
	}
}


void parseIncomingArpPacket(struct arpPacket *pkt) {

	switch(ntohs(pkt->oper)) {
	case 0x0001: // ARP Request
		rxArpRequest(pkt);
		break;
	case 0x0002: // ARP Reply
		rxArpReply(pkt);
		break;
	}
}


