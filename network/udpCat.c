
#include <LPC17xx.h>
#include "bitTypes.h"

#include "debug.h"

#include "emac.h"
#include "network/endian.h"
#include "network/ethernet.h"
#include "network/ip.h"
#include "network/udp.h"


/*
 *  Send simple ASCII strings over UDP packets.  This is for sending debugging messages
 *  to a workstation once the network is working.  
 */

// Where shall we send our messages to?
const char nc_endpoint_macAddr[6] = {0xe0, 0xcb, 0x4e, 0x47, 0x7f, 0x9b};
const uint32_t nc_endpoint_ipAddr = 0xc80ba8c0; // Big-endian
const uint16_t nc_endpoint_destPort = 0x1234; // Little-endian
const uint16_t nc_endpoint_srcPort = 0x1234; // Little-endian
const char nc_endpoint_connected = 1;



struct ethernetFrame *nc_makeIPv4Packet(uint32_t destIpAddrBE) {
	unsigned int i;

	struct ethernetFrame *frame;
	struct ipPacket *ip;

	frame = ethernetGetNextTxBuffer(0x0800);
	ip = (struct ipPacket*) &frame->payload;

	for(i=0; i<6; i++) {
		frame->src[i] = myMacAddress[i];
		frame->dest[i] = nc_endpoint_macAddr[i];
	}

	ip->srcIpAddr = myIpAddress_longBE;   // <-- TODO:  look this up in ARP
	ip->destIpAddr = destIpAddrBE;

	ip->version = 0x45;
	ip->diffServicesField = 0;
	ip->identification = 0; // necessary?  other uses?
	ip->flagsAndFragOffset = 0x40; // don't fragment, no offset
	ip->ttl = 64;

	return frame;
}


void nc_finishAndSendUdpPacket(struct ethernetFrame *frame, unsigned int udpDataLen) {

	struct ipPacket *ip;
	struct udpPacket *udp;

	ip = (struct ipPacket*) &frame->payload;
	udp = (struct udpPacket*) &ip->data;

	udp->length = htons(8 + udpDataLen);
	ip->totalLength = htons(20 + 8 + udpDataLen);

	udp->checksum = 0;
	ip->headerChecksum = 0;
	ip->headerChecksum = internetChecksum(ip, 20);

	ethernetPleaseSend(frame, (14 + 20 + 8 + udpDataLen));
}



void nc(char *msg) {
	unsigned int i;

	struct ethernetFrame *frame;
	struct ipPacket *ip;
	struct udpPacket *udp;

	if (!nc_endpoint_connected) {
		return;
	}

	frame = nc_makeIPv4Packet(nc_endpoint_ipAddr);
	ip = (struct ipPacket*) &frame->payload;
	ip->protocol = 17; // UDP over IP
	udp = (struct udpPacket*) &ip->data;

	udp->srcPort = htons(nc_endpoint_srcPort);
	udp->destPort = htons(nc_endpoint_destPort);

	for (i=0; i<300; i++) {
		udp->data[i] = msg[i];
		if (msg[i] == 0) {
			break;
		}
	}

	nc_finishAndSendUdpPacket(frame, i);
}


// These functions behave identically to the debug* functions in
// debug.c:

void ncDebug(char *msg) {
	unsigned int i;

	struct ethernetFrame *frame;
	struct ipPacket *ip;
	struct udpPacket *udp;

	if (!nc_endpoint_connected) {
		return;
	}

	frame = nc_makeIPv4Packet(nc_endpoint_ipAddr);
	ip = (struct ipPacket*) &frame->payload;
	ip->protocol = 17; // UDP over IP
	udp = (struct udpPacket*) &ip->data;

	udp->srcPort = htons(nc_endpoint_srcPort);
	udp->destPort = htons(nc_endpoint_destPort);

	for (i=0; i<300; i++) {
		udp->data[i] = msg[i];
		if (msg[i] == 0) {
			break;
		}
	}

	udp->data[i++] = 0x0d; // CR
	udp->data[i++] = 0x0a; // LF

	nc_finishAndSendUdpPacket(frame, i);
}


void ncDebugByte(char *msg, uint8_t value) {
	unsigned int i;

	char buf[2];
	int bufLen = bufLen;

	struct ethernetFrame *frame;
	struct ipPacket *ip;
	struct udpPacket *udp;

	if (!nc_endpoint_connected) {
		return;
	}

	frame = nc_makeIPv4Packet(nc_endpoint_ipAddr);
	ip = (struct ipPacket*) &frame->payload;
	ip->protocol = 17; // UDP over IP
	udp = (struct udpPacket*) &ip->data;

	udp->srcPort = htons(nc_endpoint_srcPort);
	udp->destPort = htons(nc_endpoint_destPort);

	for (i=0; i<300; i++) {
		udp->data[i] = msg[i];
		if (msg[i] == 0) {
			break;
		}
	}

	udp->data[i++] = '0';
	udp->data[i++] = 'x';

	bufLen = formatHex(buf, value);

	udp->data[i++] = buf[0];
	udp->data[i++] = buf[1];

	udp->data[i++] = 0x0d; // CR
	udp->data[i++] = 0x0a; // LF

	nc_finishAndSendUdpPacket(frame, i);
}


void ncDebugWord(char *msg, uint16_t value) {
	unsigned int i;

	char buf[4];
	int bufLen = bufLen;

	struct ethernetFrame *frame;
	struct ipPacket *ip;
	struct udpPacket *udp;

	if (!nc_endpoint_connected) {
		return;
	}

	frame = nc_makeIPv4Packet(nc_endpoint_ipAddr);
	ip = (struct ipPacket*) &frame->payload;
	ip->protocol = 17; // UDP over IP
	udp = (struct udpPacket*) &ip->data;

	udp->srcPort = htons(nc_endpoint_srcPort);
	udp->destPort = htons(nc_endpoint_destPort);

	for (i=0; i<300; i++) {
		udp->data[i] = msg[i];
		if (msg[i] == 0) {
			break;
		}
	}

	udp->data[i++] = '0';
	udp->data[i++] = 'x';

	bufLen = formatHexWord(buf, value);

	udp->data[i++] = buf[0];
	udp->data[i++] = buf[1];
	udp->data[i++] = buf[2];
	udp->data[i++] = buf[3];

	udp->data[i++] = 0x0d; // CR
	udp->data[i++] = 0x0a; // LF

	nc_finishAndSendUdpPacket(frame, i);
}


void ncDebugLong(char *msg, uint32_t value) {
	unsigned int i;

	char buf[8];
	int bufLen = bufLen;

	struct ethernetFrame *frame;
	struct ipPacket *ip;
	struct udpPacket *udp;

	if (!nc_endpoint_connected) {
		return;
	}

	frame = nc_makeIPv4Packet(nc_endpoint_ipAddr);
	ip = (struct ipPacket*) &frame->payload;
	ip->protocol = 17; // UDP over IP
	udp = (struct udpPacket*) &ip->data;

	udp->srcPort = htons(nc_endpoint_srcPort);
	udp->destPort = htons(nc_endpoint_destPort);

	for (i=0; i<300; i++) {
		udp->data[i] = msg[i];
		if (msg[i] == 0) {
			break;
		}
	}

	udp->data[i++] = '0';
	udp->data[i++] = 'x';

	bufLen = formatHexLong(buf, value);

	udp->data[i++] = buf[0];
	udp->data[i++] = buf[1];
	udp->data[i++] = buf[2];
	udp->data[i++] = buf[3];
	udp->data[i++] = buf[4];
	udp->data[i++] = buf[5];
	udp->data[i++] = buf[6];
	udp->data[i++] = buf[7];

	udp->data[i++] = 0x0d; // CR
	udp->data[i++] = 0x0a; // LF

	nc_finishAndSendUdpPacket(frame, i);
}
