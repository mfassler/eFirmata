
#ifndef __UDP_H_
#define __UDP_H_


// For *incoming firmata-over-udp packets*, the first 8 bytes MUST
// always be "eFirmata" (so that we don't react to random garbage
// on the net.
#define _FIRMATA_ID_TOKEN "eFirmata"
extern const char FIRMATA_ID_TOKEN[];


struct udpPacket {
	uint16_t srcPort;
	uint16_t destPort;
	uint16_t length;
	uint16_t checksum;
	char data[];
};

extern void parseIncomingUdpPacket(struct ethernetFrame *, unsigned int);
extern void udpEcho(struct ethernetFrame *, unsigned int);
extern void udpToDebug(char *, unsigned short);
extern struct ethernetFrame *udp_makeAndPrepareUdpPacket(uint32_t, char *, uint16_t, uint16_t);
extern void udp_finishAndSendUdpPacket(struct ethernetFrame *, unsigned int);


#endif // __UDP_H_

