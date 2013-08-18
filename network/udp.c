
#include "debug.h"

#include "network/ip.h"
#include "network/udp.h"
#include "network/udpServices/oscope.h"
#include "network/endian.h"
#include "emac.h"

// For incoming firmata-over-UDP packets, the first 8 bytes MUST
// always be "eFirmata":
const char FIRMATA_ID_TOKEN[8] = _FIRMATA_ID_TOKEN;


void parseIncomingUdpPacket(struct ethernetFrame *frame, unsigned int length) {
	struct ipPacket *ip;
	struct udpPacket *udp;
	unsigned short dataLen, udpDataLen;
	uint16_t myPort;

	ip = (struct ipPacket *) &frame->payload;
	udp = (struct udpPacket *) &ip->data;

	// The EMAC is setup to pad all frames to 60 bytes minimum, so this
	// will be 18 bytes, minimum...
	dataLen = length - ((unsigned long)&udp->data - (unsigned long)frame);
	// The UDP header *should* have the correct number for us:
	udpDataLen = ntohs(udp->length) - 8;
	if (udpDataLen < dataLen) {
		dataLen = udpDataLen;
	}

	myPort = ntohs(udp->destPort);

	switch (myPort) {
		case 2112:
			udpToDebug(udp->data, dataLen);
			break;
		case 2113:
			udpEcho(frame, length);
			break;
		case 2114:
			incomingOscopeOverUdp(frame, length);
			break;
		default:
			debugWord("UDP Packet, port: ", myPort);
			break;
	}

}

void udpEcho(struct ethernetFrame *request, unsigned int length) {
	struct ethernetFrame *reply;
	struct ipPacket *requestIp, *replyIp;
	struct udpPacket *requestUdp, *replyUdp;
	unsigned short ipLen, i;

	requestIp = (struct ipPacket*) &request->payload;
	requestUdp = (struct udpPacket*) &requestIp->data;

	reply = ethernetGetNextTxBuffer(0x0800);
	replyIp = (struct ipPacket*) &reply->payload;
	replyUdp = (struct udpPacket*) &replyIp->data;

	ipLen = length - ((unsigned long)requestIp - (unsigned long)request);

	// First, just copy the entire IP packet:
	for (i=0; i< ipLen; i++) {
		((char*) replyIp)[i] = ((char*) requestIp)[i];
	}

	// Set the receiver (ethernet frame)  (sender should already be set)
	reply->dest[0] = request->src[0];
	reply->dest[1] = request->src[1];
	reply->dest[2] = request->src[2];
	reply->dest[3] = request->src[3];
	reply->dest[4] = request->src[4];
	reply->dest[5] = request->src[5];

	// Swap sender and receiver (IP packet):
	replyIp->srcIpAddr = requestIp->destIpAddr;
	replyIp->destIpAddr = requestIp->srcIpAddr;

	// Swap the ports:
	replyUdp->srcPort = requestUdp->destPort;
	replyUdp->destPort = requestUdp->srcPort;

	ethernetPleaseSend(reply, length);
}

void udpToDebug(char * data, unsigned short length) {

	unsigned short i;
	char buffer[100];

	for (i = 0; i<length; i++) {
		buffer[i] = data[i];
		if (i > 98) {
			break;
		}
	}
	i++;
	buffer[i] = 0;

	debug(buffer);
}


