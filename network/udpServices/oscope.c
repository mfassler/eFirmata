
#include "debug.h"

#include "network/ethernet.h"
#include "network/ip.h"
#include "network/udp.h"
#include "network/udpServices/oscope.h"
#include "network/endian.h"

#include "adc.h"


void incomingOscopeOverUdp(struct ethernetFrame *frame, unsigned int length) {
	const char FIRMATA_ID_SUBTOKEN[3] = "TOS"; // This is TriggeredOscilloScope
	const uint8_t FIRMATA_TOS_VERSION = 0;

	struct ipPacket *ip;
	struct udpPacket *udp;
	struct scopeCmdOverUdp *cmd;
	unsigned short i;

	ip = (struct ipPacket*) &frame->payload;
	udp = (struct udpPacket*) &ip->data;
	cmd = (struct scopeCmdOverUdp*) &udp->data;

	// Check that this packet starts with the string "eFirmata"
	for (i=0; i<8; i++) {
		if (cmd->idToken[i] != FIRMATA_ID_TOKEN[i]) {
			return;
		}
	}

	// Check that this is for Triggered OscilloScope:
	for (i=0; i<3; i++) {
		if (cmd->idSubToken[i] != FIRMATA_ID_SUBTOKEN[i]) {
			return;
		}
	}

	// We only support protocol version 0:
	if (cmd->version != FIRMATA_TOS_VERSION) {
		return;
	}

	triggerChannel = cmd->triggerChannel;
	triggerLevel = cmd->triggerLevel;

	triggerNumSamplesReq = ntohl(cmd->triggerNumSamplesReq); // Number of samples requested

	switch(cmd->triggerMode) {
		case TRIGGERMODE_OFF:
			adc_weAreSending = 0;
			triggerEnabled = 0;
			break;
		case TRIGGERMODE_NOW:
			prepOutgoingFastBuffers(frame, length);
			adc_weAreSending = 1;
			break;
		case TRIGGERMODE_RISING:
			prepOutgoingFastBuffers(frame, length);
			triggerDirection = 1;
			triggerEnabled = 1;
			break;
		case TRIGGERMODE_FALLING:
			prepOutgoingFastBuffers(frame, length);
			triggerDirection = 0;
			triggerEnabled = 1;
			break;
		//case TRIGGERMODE_CONTINUOUS:
			//  break;
		default:
			adc_weAreSending = 0;
			triggerEnabled = 0;
		break;
	}
}


extern struct ethernetFrame *bigEtherFrameA;
extern struct ethernetFrame *bigEtherFrameB;

void prepOutgoingFastBuffers(struct ethernetFrame *inFrame, unsigned int inFrameLength) {
	struct ipPacket *inIp, *outIpA, *outIpB;
	struct udpPacket *inUdp, *outUdpA, *outUdpB;
	unsigned short ipLen, i;

	inIp = (struct ipPacket*) &inFrame->payload;
	outIpA = (struct ipPacket*) &bigEtherFrameA->payload;
	outIpB = (struct ipPacket*) &bigEtherFrameB->payload;

	inUdp = (struct udpPacket*) &inIp->data;
	outUdpA = (struct udpPacket*) &outIpA->data;
	outUdpB = (struct udpPacket*) &outIpB->data;

	ipLen = inFrameLength - ((unsigned long)inIp - (unsigned long)inFrame);

	// First, just copy the entire IP packet:
	for (i=0; i<ipLen; i++) {
		((char*) outIpA)[i] = ((char*) inIp)[i];
		((char*) outIpB)[i] = ((char*) inIp)[i];
	}

	// Set the dest MAC address (ethernet).  (our src MAC should already be set)
	bigEtherFrameA->dest[0] = inFrame->src[0];
	bigEtherFrameA->dest[1] = inFrame->src[1];
	bigEtherFrameA->dest[2] = inFrame->src[2];
	bigEtherFrameA->dest[3] = inFrame->src[3];
	bigEtherFrameA->dest[4] = inFrame->src[4];
	bigEtherFrameA->dest[5] = inFrame->src[5];

	bigEtherFrameB->dest[0] = inFrame->src[0];
	bigEtherFrameB->dest[1] = inFrame->src[1];
	bigEtherFrameB->dest[2] = inFrame->src[2];
	bigEtherFrameB->dest[3] = inFrame->src[3];
	bigEtherFrameB->dest[4] = inFrame->src[4];
	bigEtherFrameB->dest[5] = inFrame->src[5];

	// Swap source and dest IP addresses (IP packet):
	outIpA->srcIpAddr = inIp->destIpAddr;
	outIpA->destIpAddr = inIp->srcIpAddr;

	outIpB->srcIpAddr = inIp->destIpAddr;
	outIpB->destIpAddr = inIp->srcIpAddr;

	// Swap the UDP ports
	outUdpA->srcPort = inUdp->destPort;
	outUdpA->destPort = inUdp->srcPort;

	outUdpB->srcPort = inUdp->destPort;
	outUdpB->destPort = inUdp->srcPort;

	// IPv4 lets us ignore UDP checksums:
	outUdpA->udpChecksum = 0;
	outUdpB->udpChecksum = 0;
}




