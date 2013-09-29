
#ifndef __FIRMATA_SPI_OVER_UDP_H_
#define __FIRMATA_SPI_OVER_UDP_H_

#include "network/ethernet.h"

struct spiCmdOverUdp {
	char idToken[8]; // For *incoming udp* must always be: "eFirmata" (so that we ignore random crap)
	char idSubToken[3]; // For incoming UDP, must always be:  "SPI"
	uint8_t version; // Protocol version.  Right now, only 0x00 is allowed. 
	uint32_t extra; // This will be for flags, options, etc.  Ignored for now.  Set to zeros.
	char mode; // bit1->CPOL, bit0->CPHA
	uint8_t numBytes;
	char data[];
};


extern void udpSPI(struct ethernetFrame *, unsigned int);
extern void udpSPI_replyToSender(volatile uint16_t *, uint8_t, uint8_t);


#endif // __FIRMATA_SPI_OVER_UDP_H_

