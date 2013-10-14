
#include <LPC17xx.h>
#include "bitTypes.h"

#include "debug.h"

#include "emac.h"
#include "network/endian.h"
#include "network/ethernet.h"
#include "network/ip.h"
#include "network/udp.h"
#include "network/udpCat.h"

#include "network/udpServices/firmataSPI.h"

#include "ssp.h"


// Whoever last sent us data gets the return data:
static char spi_endpoint_macAddr[6];
static uint32_t spi_endpoint_ipAddr;
static uint16_t spi_endpoint_srcPort;
static uint16_t spi_endpoint_destPort;
static char spi_endpoint_connected = 0;


void udpSPI(struct ethernetFrame *frame, unsigned int length) {
	const char FIRMATA_ID_SUBTOKEN[3] = "SPI"; // This is SPI
	const uint8_t FIRMATA_PWM_VERSION = 0;

	struct ipPacket *ip;
	struct udpPacket *udp;
	struct spiCmdOverUdp *cmd;
	unsigned short i;

	(void)length;  // Unused parameter

	ip = (struct ipPacket*) &frame->payload;
	udp = (struct udpPacket*) &ip->data;
	cmd = (struct spiCmdOverUdp*) &udp->data;

	// Check that this packet starts with the string "eFirmata"
	for (i=0; i<8; i++) {
		if (cmd->idToken[i] != FIRMATA_ID_TOKEN[i]) {
			return;
		}
	}

	// Check that this is for SPI:
	for (i=0; i<3; i++) {
		if (cmd->idSubToken[i] != FIRMATA_ID_SUBTOKEN[i]) {
			return;
		}
	}

	// We only support protocol version 0:
	if (cmd->version != FIRMATA_PWM_VERSION) {
		return;
	}

	for (i=0; i<6; i++) {
		spi_endpoint_macAddr[i] = frame->src[i];
	}
	spi_endpoint_ipAddr = ip->srcIpAddr;
	spi_endpoint_destPort = udp->srcPort;
	spi_endpoint_srcPort = udp->destPort;
	spi_endpoint_connected = 1;

	// UDP header is 8 bytes.  SPI-over-UDP header is 16 bytes
	SSP0_pleaseSend(cmd->data, ntohs(udp->length) - 24);
}


// This is for use with the SSP ring buffer.
void udpSPI_replyToSender(volatile uint16_t *ringBuffer, uint8_t startIdx, uint8_t numBytes) {
	struct ethernetFrame *frame;
	struct ipPacket *ip;
	struct udpPacket *udp;
	uint8_t bufIdx; // must wrap around after 255
	int i;

	if (!spi_endpoint_connected) {
		return;
	}

	frame = ethernetGetNextTxBuffer(0x0800);
	ip = (struct ipPacket*) &frame->payload;
	udp = (struct udpPacket*) &ip->data;

	for(i=0; i<6; i++) {
		frame->src[i] = myMacAddress[i];
		frame->dest[i] = spi_endpoint_macAddr[i];
	}

	ip->srcIpAddr = myIpAddress_longBE;
	ip->destIpAddr = spi_endpoint_ipAddr;

	ip->version = 0x45;
	ip->diffServicesField = 0;
	ip->identification = 0; // necessary?  other uses?
	ip->flagsAndFragOffset = 0x40; // don't fragment, no offset
	ip->ttl = 64;
	ip->protocol = 17;

	udp->srcPort = spi_endpoint_srcPort;
	udp->destPort = spi_endpoint_destPort;

	// UDP header is 8 bytes.  IP header is 20 bytes
	udp->length = htons(numBytes + 8);
	ip->totalLength = htons(numBytes + 8 + 20);

	for (i = 0; i<numBytes; i++) {
		bufIdx = startIdx + i;
		udp->data[i] = (char) ringBuffer[bufIdx];
	}

	udp->checksum = 0;  // optional, so fuckit.
	ip->headerChecksum = 0; // Needed to calculate the actual checksum
	ip->headerChecksum = internetChecksum(ip, 20);

	ethernetPleaseSend(frame, (14 + 20 + 8 + numBytes));

}


