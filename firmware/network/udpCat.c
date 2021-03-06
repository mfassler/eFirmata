
#include <LPC17xx.h>
#include "bitTypes.h"

#include "debug.h"

#include "emac.h"
#include "network/endian.h"
#include "network/ethernet.h"
#include "network/ip.h"
#include "network/arp.h"
#include "network/udp.h"


/*
 *  Send simple ASCII strings over UDP packets.  This is for sending debugging messages
 *  to a workstation once the network is working.  
 */



// Where shall we send our messages to?
#include "network/DEFAULT_IP_ADDRESS.h"

static uint32_t nc_endpoint_ipAddrBE;
static uint16_t nc_endpoint_destPort = NC_DEBUG_PORT;
static char nc_endpoint_connected = NC_DEBUG_CONNECTED;

void nc_ipaddr_init(void) {
	unsigned char nc_ip_addr[4] = NC_DEBUG_IP_ADDR;

	nc_endpoint_ipAddrBE = (nc_ip_addr[0]) |
	                       (nc_ip_addr[1] << 8) |
	                       (nc_ip_addr[2] << 16) |
	                       (nc_ip_addr[3] << 24);
}



struct ethernetFrame *nc_makePacket(void) {

	uint16_t srcPort = 0x1234;  // local-endian (little-endian)
	char nc_endpoint_macAddr[6];

	arpCacheLookup(nc_endpoint_ipAddrBE, nc_endpoint_macAddr);

	return udp_makeAndPrepareUdpPacket(
		nc_endpoint_ipAddrBE,
		nc_endpoint_macAddr,
		nc_endpoint_destPort,
		srcPort
	);
}




void nc(char *msg) {
	unsigned int i;

	struct ethernetFrame *frame;
	struct ipPacket *ip;
	struct udpPacket *udp;

	if (!nc_endpoint_connected) {
		return;
	}

	frame = nc_makePacket();
	ip = (struct ipPacket*) &frame->payload;
	udp = (struct udpPacket*) &ip->data;

	for (i=0; i<300; i++) {
		udp->data[i] = msg[i];
		if (msg[i] == 0) {
			break;
		}
	}

	udp_finishAndSendUdpPacket(frame, i);
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

	frame = nc_makePacket();
	ip = (struct ipPacket*) &frame->payload;
	udp = (struct udpPacket*) &ip->data;

	for (i=0; i<300; i++) {
		udp->data[i] = msg[i];
		if (msg[i] == 0) {
			break;
		}
	}

	udp->data[i++] = 0x0d; // CR
	udp->data[i++] = 0x0a; // LF

	udp_finishAndSendUdpPacket(frame, i);
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

	frame = nc_makePacket();
	ip = (struct ipPacket*) &frame->payload;
	udp = (struct udpPacket*) &ip->data;

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

	udp_finishAndSendUdpPacket(frame, i);
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

	frame = nc_makePacket();
	ip = (struct ipPacket*) &frame->payload;
	udp = (struct udpPacket*) &ip->data;

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

	udp_finishAndSendUdpPacket(frame, i);
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

	frame = nc_makePacket();
	ip = (struct ipPacket*) &frame->payload;
	udp = (struct udpPacket*) &ip->data;

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

	udp_finishAndSendUdpPacket(frame, i);
}

