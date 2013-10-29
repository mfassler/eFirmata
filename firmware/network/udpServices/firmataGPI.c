
#include <LPC17xx.h>
#include "bitTypes.h"

#include "debug.h"

#include "emac.h"
#include "network/endian.h"
#include "network/ethernet.h"
#include "network/ip.h"
#include "network/udp.h"
#include "network/udpCat.h"

#include "network/udpServices/firmataGPI.h"

#include "ssp.h"


// Whoever last sent us a command gets the return data:
static char gpi_endpoint_macAddr[6];
static uint32_t gpi_endpoint_ipAddrBE;
static uint16_t gpi_endpoint_srcPort;
static uint16_t gpi_endpoint_destPort;
static char gpi_endpoint_connected = 0;


void udpGPI(struct ethernetFrame *frame, unsigned int length) {
	const char FIRMATA_ID_SUBTOKEN[3] = "GPI"; // This is GPIO Input
	const uint8_t FIRMATA_GPI_VERSION = 0;

	struct ipPacket *ip;
	struct udpPacket *udp;
	struct gpiCmdOverUdp *cmd;
	unsigned short i;

	(void)length;  // Unused parameter

	ip = (struct ipPacket*) &frame->payload;
	udp = (struct udpPacket*) &ip->data;
	cmd = (struct gpiCmdOverUdp*) &udp->data;

	// Check that this packet starts with the string "eFirmata"
	for (i=0; i<8; i++) {
		if (cmd->idToken[i] != FIRMATA_ID_TOKEN[i]) {
			return;
		}
	}

	// Check that this is for GPI:
	for (i=0; i<3; i++) {
		if (cmd->idSubToken[i] != FIRMATA_ID_SUBTOKEN[i]) {
			return;
		}
	}

	// We only support protocol version 0:
	if (cmd->version != FIRMATA_GPI_VERSION) {
		return;
	}

	for (i=0; i<6; i++) {
		gpi_endpoint_macAddr[i] = frame->src[i];
	}
	gpi_endpoint_ipAddrBE = ip->srcIpAddr;
	gpi_endpoint_destPort = ntohs(udp->srcPort);
	gpi_endpoint_srcPort = ntohs(udp->destPort);
	gpi_endpoint_connected = 1;

	udpGPI_replyToSender();
}


void udpGPI_replyToSender(void) {
	struct ethernetFrame *frame;
	struct ipPacket *ip;
	struct udpPacket *udp;

	uint32_t * data;

	if (!gpi_endpoint_connected) {
		return;
	}

	frame = udp_makeAndPrepareUdpPacket(
		gpi_endpoint_ipAddrBE,
		gpi_endpoint_macAddr,
		gpi_endpoint_srcPort,
		gpi_endpoint_destPort);

	ip = (struct ipPacket*) &frame->payload;
	udp = (struct udpPacket*) &ip->data;

	data = (uint32_t *)udp->data;

	data[0] = htonl(LPC_GPIO0->FIOPIN);
	data[1] = htonl(LPC_GPIO1->FIOPIN);
	data[2] = htonl(LPC_GPIO2->FIOPIN);
	data[3] = htonl(LPC_GPIO3->FIOPIN);
	data[4] = htonl(LPC_GPIO4->FIOPIN);

	udp_finishAndSendUdpPacket(frame, 20);

}


