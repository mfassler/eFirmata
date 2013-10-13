
#include "debug.h"
#include "network/udpCat.h"

#include "network/ethernet.h"
#include "network/ip.h"
#include "network/udp.h"
#include "network/udpServices/oscope.h"
#include "network/endian.h"

#include "adc.h"


void incomingOscopeOverUdp(struct ethernetFrame *frame, unsigned int length) {
	const char FIRMATA_ID_SUBTOKEN[3] = "TOC"; // This is a "Triggered Oscilloscope Command"
	const uint8_t FIRMATA_TOC_VERSION = 0;

	struct ipPacket *ip;
	struct udpPacket *udp;
	struct scopeCmdOverUdp *cmd;
	unsigned short i;

	(void)length;  // Unused parameter

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
	if (cmd->version != FIRMATA_TOC_VERSION) {
		return;
	}

	triggerChannel = cmd->triggerChannel;
	triggerLevel = ntohl(cmd->triggerLevel) & 0xff;

	triggerNumSamplesReq = ntohl(cmd->triggerNumSamplesReq); // Number of samples requested

	switch(cmd->triggerMode) {
		case TRIGGERMODE_OFF:
			adc_weAreSending = 0;
			triggerEnabled = 0;
			break;
		case TRIGGERMODE_NOW:
			oscope_sendMetaData(ip->srcIpAddr, frame->src, ntohs(udp->destPort), ntohs(udp->srcPort));
			prepOutgoingFastBuffers(ip->srcIpAddr, frame->src, ntohs(udp->destPort), ntohs(udp->srcPort));
			adc_weAreSending = 1;
			break;
		case TRIGGERMODE_RISING:
			oscope_sendMetaData(ip->srcIpAddr, frame->src, ntohs(udp->destPort), ntohs(udp->srcPort));
			prepOutgoingFastBuffers(ip->srcIpAddr, frame->src, ntohs(udp->destPort), ntohs(udp->srcPort));
			triggerDirection = 1;
			triggerEnabled = 1;
			break;
		case TRIGGERMODE_FALLING:
			oscope_sendMetaData(ip->srcIpAddr, frame->src, ntohs(udp->destPort), ntohs(udp->srcPort));
			prepOutgoingFastBuffers(ip->srcIpAddr, frame->src, ntohs(udp->destPort), ntohs(udp->srcPort));
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


void oscope_sendMetaData(uint32_t destIpAddrBE, char *destMacAddr, uint16_t srcPort, uint16_t dstPort) {

	unsigned int i;
	uint8_t numChannels = 4;

	struct ethernetFrame *frame;
	struct ipPacket *ip;
	struct udpPacket *udp;
	struct scopeMetaData *tom;

	//ncDebug("this is oscope_sendMetaData()");

	frame = udp_makeAndPrepareUdpPacket(destIpAddrBE, destMacAddr, srcPort, dstPort);
	ip = (struct ipPacket*) &frame->payload;
	udp = (struct udpPacket*) &ip->data;
	tom = (struct scopeMetaData*) &udp->data;

	tom->idToken[0] = 'J';
	tom->idToken[1] = 'i';
	tom->idToken[2] = 'm';
	tom->idToken[3] = 'W';
	tom->idToken[4] = 'i';
	tom->idToken[5] = 'n';
	tom->idToken[6] = 'k';
	tom->idToken[7] = 's';

	tom->idSubToken[0] = 'T';
	tom->idSubToken[1] = 'O';
	tom->idSubToken[2] = 'M';

	tom->version = 0;


	// We seem to be operating at 38400 samples per second:
	tom->domainPhysicalUnit = 's';  // seconds per sample, but...
	tom->domainPhysicalUnit |= 0x80;  // invert, so it's samples per second

	tom->stepSizeDatatype = 'H';   // domainStepSize will have a datatype of uint16_t

	tom->numChannels = numChannels;
	tom->bytesPerDescriptor = sizeof(struct channelMetaData);

	* (uint16_t*)&tom->domainStepSize[6] = htons(38400);




	// Zero out the channel meta-data
	for (i=0; i< (numChannels * sizeof(struct channelMetaData)); i++) {
		((char*) tom->channels)[i] = 0;
	}


	for (i=0; i< numChannels; i++) {
		// Physical units are defined at http://physics.nist.gov    'V' is Volts
		tom->channels[i].units = 'V';

		// Datatypes are defined in Python's struct module    'B' is uint8, 'f' is 32bit float

		// On the UDP packet, we will send a single uint8_t:
		tom->channels[i].dataDataType = 'B';

		// It will be translated into, say: 1.22 volts, a Float:
		tom->channels[i].realDataType = 'f';


		tom->channels[i].scaleType = TWO_POINT_LINEAR; // two points define a line

		// When the data in the UDP packet is 0, it represents 0.0 volts:
		// (packet is already zero'd)
		//tom->channel[i]dataValA = htonl(0);  //simpler to treat as a long, even though it's only one byte

		// When the data in the UDP packet is 255, it represents 3.3 volts:
		tom->channels[i].dataValB = htonl(255); // simpler to treat as a long, even though it's only one byte
		// (Manually construct a 32bit float, because this is an INT-only processor)
		tom->channels[i].realValB[4] = 64;
		tom->channels[i].realValB[5] = 83;
		tom->channels[i].realValB[6] = 51;
		tom->channels[i].realValB[7] = 51;

	}

	// Error type is not used yet.  This is just random stuff:
	tom->channels[0].errorType = 'Q';
	tom->channels[1].errorType = 'R';
	tom->channels[2].errorType = 'S';
	tom->channels[3].errorType = 'T';

	udp_finishAndSendUdpPacket(frame, sizeof(struct scopeMetaData) + sizeof(struct channelMetaData) * 4);
}


extern struct ethernetFrame *bigEtherFrameA;
extern struct ethernetFrame *bigEtherFrameB;

//void prepOutgoingFastBuffers(struct ethernetFrame *inFrame, unsigned int inFrameLength) {
void prepOutgoingFastBuffers(uint32_t destIpAddr, char* destMacAddr, uint16_t srcPort, uint16_t destPort) {
	struct ipPacket *ipA, *ipB;
	struct udpPacket *udpA, *udpB;
	struct scopeData *todA, *todB;
	unsigned short i;

	ipA = (struct ipPacket*) &bigEtherFrameA->payload;
	udpA = (struct udpPacket*) &ipA->data;
	todA = (struct scopeData*) &udpA->data;

	ipB = (struct ipPacket*) &bigEtherFrameB->payload;
	udpB = (struct udpPacket*) &ipB->data;
	todB = (struct scopeData*) &udpB->data;

	//ipLen = inFrameLength - ((unsigned long)inIp - (unsigned long)inFrame);


	// Set the dest MAC address (ethernet).  (our src MAC should already be set)
	for (i=0; i<6; i++) {
		// src MAC address should alread be set...
		bigEtherFrameA->src[i] = myMacAddress[i];
		bigEtherFrameB->src[i] = myMacAddress[i];

		bigEtherFrameA->dest[i] = destMacAddr[i];
		bigEtherFrameB->dest[i] = destMacAddr[i];
	}

	// Set the src and dest IP address;
	ipA->srcIpAddr = myIpAddress_longBE;
	ipA->destIpAddr = destIpAddr;

	ipB->srcIpAddr = myIpAddress_longBE;
	ipB->destIpAddr = destIpAddr;


	// Make the headers
	ipA->version = 0x45;
	ipA->diffServicesField = 0;
	ipA->identification = 0; // necessary?  other uses?
	ipA->flagsAndFragOffset = 0x40; // don't fragment, no offset
	ipA->ttl = 64;
	ipA->protocol = 17; // UDP over IP

	ipB->version = 0x45;
	ipB->diffServicesField = 0;
	ipB->identification = 0; // necessary?  other uses?
	ipB->flagsAndFragOffset = 0x40; // don't fragment, no offset
	ipB->ttl = 64;
	ipB->protocol = 17; // UDP over IP


	// Set the UDP ports
	udpA->srcPort = htons(srcPort);
	udpA->destPort = htons(destPort);
	udpB->srcPort = htons(srcPort);
	udpB->destPort = htons(destPort);

	todA->idToken[0] = 'J';
	todA->idToken[1] = 'i';
	todA->idToken[2] = 'm';
	todA->idToken[3] = 'W';
	todA->idToken[4] = 'i';
	todA->idToken[5] = 'n';
	todA->idToken[6] = 'k';
	todA->idToken[7] = 's';

	todA->idSubToken[0] = 'T';
	todA->idSubToken[1] = 'O';
	todA->idSubToken[2] = 'D';

	todA->version = 0;

	todB->idToken[0] = 'J';
	todB->idToken[1] = 'i';
	todB->idToken[2] = 'm';
	todB->idToken[3] = 'W';
	todB->idToken[4] = 'i';
	todB->idToken[5] = 'n';
	todB->idToken[6] = 'k';
	todB->idToken[7] = 's';

	todB->idSubToken[0] = 'T';
	todB->idSubToken[1] = 'O';
	todB->idSubToken[2] = 'D';

	todB->version = 0;


}




