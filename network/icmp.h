

#ifndef __ICMP_
#define __ICMP_

struct icmpPacket {
	uint8_t type;
	uint8_t code;
	uint16_t checksum;
	uint32_t restOfHeader;
};


extern void parseIncomingIcmpPacket(struct ethernetFrame *, unsigned int);
extern void icmp_answerPing(struct ethernetFrame *, unsigned int );
extern uint16_t internetChecksum(void *, unsigned int);


#endif // __ICMP_
