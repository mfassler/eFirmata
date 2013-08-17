
#ifndef __ARP_H
#define __ARP_H

//char myIpAddress[4] = {192, 168, 11, 169};
//char myMacAddress[6] = SELF_ADDR;

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

extern void parseIncomingArpPacket(struct arpPacket *);

#define SIZE_OF_ARP_IN_ETHERNET 42 // Ethernet header + sizeof(struct arpPacket)
// The current ethernet setup will pad all frames to 60 bytes minimum, regardless...


#endif // __ARP_H

