
#ifndef __ETHERNET_PROTOCOL_
#define __ETHERNET_PROTOCOL_

struct ethernetFrame {
	char dest[6];
	char src[6];
	char type[2];

	uint8_t payload[1500];

	// data can be shorter than this, and so fcs will be closer up...

	uint32_t fcs;
};

extern void parseFrame(char*, unsigned short);
extern void ethernetInitTxBuffers(void);
extern struct ethernetFrame *ethernetGetNextTxBuffer(uint16_t);


#endif  // __ETHERNET_PROTOCOL_

