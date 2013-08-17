
#ifndef __UDP_
#define __UDP_

struct udpPacket {
	uint16_t srcPort;
	uint16_t destPort;
	uint16_t length;
	uint16_t udpChecksum;
	char data;
};

extern void parseIncomingUdpPacket(struct ethernetFrame *, unsigned int);
extern void udpEcho(struct ethernetFrame *, unsigned int);
extern void udpToDebug(char *, unsigned short);

#endif // __UDP_

