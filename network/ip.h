

#ifndef __IP_PROTO_
#define __IP_PROTO_

#include "network/ethernet.h"

struct ipPacket {
	uint8_t version;  // 4bit version, 4bit header length
	uint8_t diffServicesField; // 6bit DSCP and 3bit ECN
	uint16_t totalLength;
	uint16_t identification;
	uint16_t flagsAndFragOffset; // 3bit flags, 13bit fragment offset
	uint8_t ttl;
	uint8_t protocol; // UDP or TCP or ICMP, etc.
	uint16_t headerChecksum;
	uint32_t srcIpAddr;
	uint32_t destIpAddr;
	uint8_t data[];
};

extern volatile char myIpAddress_char[];
extern volatile uint32_t myIpAddress_longBE;


extern void parseIncomingIpPacket(struct ethernetFrame *, unsigned int);
extern uint16_t internetChecksum(void *, unsigned int);
extern void setIpAddress(char*);

#endif // __IP_PROTO_
