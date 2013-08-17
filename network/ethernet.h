
#ifndef __ETHERNET_PROTOCOL_
#define __ETHERNET_PROTOCOL_

struct ethernetFrame {
	char dest[6];
	char src[6];
	uint16_t type;

	uint8_t payload[1500];

	// data can be shorter than this, and so fcs will be closer up...

	uint32_t fcs;
};

extern void parseFrame(struct ethernetFrame*, unsigned int);
extern void ethernetInitTxBuffers(void);
extern struct ethernetFrame *ethernetGetNextTxBuffer(uint16_t);


#endif  // __ETHERNET_PROTOCOL_

