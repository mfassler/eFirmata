
#ifndef __ARP_H
#define __ARP_H


struct arpEntry {
	uint8_t macAddress[6];
	uint32_t ipAddress;
};

struct arpEntry arpTable[10];

struct arpPacket {
	uint16_t htype; // Always 1 for Ethernet
	uint16_t ptype; // Always 0x800 for IP (Internet Protocol)
	uint8_t hlen; // always 6 for ethernet
	uint8_t plen; // always 4 for IPv4
	uint16_t oper; // == 1 for request, 2 for reply
	uint8_t senderMacAddress[6];
	uint8_t senderIpAddress[4];
	uint8_t targetMacAddress[6];
	uint8_t targetIpAddress[4];
};

extern void initArpCache();
extern void parseIncomingArpPacket(struct arpPacket *);
extern void txArpRequest(uint32_t);
extern int arpCacheLookup(uint32_t, char*);


#define SIZE_OF_ARP_IN_ETHERNET 42 // Ethernet header + sizeof(struct arpPacket)
// The current ethernet setup will pad all frames to 60 bytes minimum, regardless...

struct arpCacheEntry {
	uint32_t ipAddrBE;
	uint32_t timestamp; // in milli-seconds
	char macAddr[6];
	char status; // 0==nothing, 1==reqSent, 2==replyReceived
};


#endif // __ARP_H

