
#ifndef __ETHERNET_PROTOCOL_H_
#define __ETHERNET_PROTOCOL_H_

// Ethertypes for the eFirmata Protocol (firmata over ethernet)
#define EFIRMATA_PROTOCOL_CONTROL 0x181b
#define EFIRMATA_PROTOCOL 0x181c
#define EFIRMATA_PROTOCOL_FAST 0x181d


struct ethernetFrame {
	char dest[6];
	char src[6];
	uint16_t type;

	uint8_t payload[1500];

};

extern void parseFrame(struct ethernetFrame*, unsigned int);
extern void ethernetInitTxBuffers(void);
extern struct ethernetFrame *ethernetGetNextTxBuffer(uint16_t);
extern void setMacAddress(void);


#endif  // __ETHERNET_PROTOCOL_H_

