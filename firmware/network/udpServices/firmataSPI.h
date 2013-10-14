
#ifndef __FIRMATA_SPI_OVER_UDP_H_
#define __FIRMATA_SPI_OVER_UDP_H_

#include "network/ethernet.h"

struct spiCmdOverUdp {
	char idToken[8]; // For *incoming udp* must always be: "eFirmata" (so that we ignore random crap)
	char idSubToken[3]; // For incoming UDP, must always be:  "SPI"
	uint8_t version; // Protocol version.  Right now, only 0x00 is allowed. 
	char extra[3];
	uint8_t flags;  // bit2 is "wait", bit1 is "CPOL", bit0 is "CPHA"
	char data[];
};


extern void udpSPI(struct ethernetFrame *, unsigned int);
extern void udpSPI_replyToSender(volatile uint16_t *, uint8_t, uint8_t);


#endif // __FIRMATA_SPI_OVER_UDP_H_

