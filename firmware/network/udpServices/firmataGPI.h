
#ifndef __FIRMATA_GPI_OVER_UDP_H_
#define __FIRMATA_GPI_OVER_UDP_H_

#include "network/ethernet.h"

struct gpiCmdOverUdp {
	char idToken[8]; // For *incoming udp* must always be: "eFirmata" (so that we ignore random crap)
	char idSubToken[3]; // For incoming UDP, must always be:  "SPI"
	uint8_t version; // Protocol version.  Right now, only 0x00 is allowed. 
};


extern void udpGPI(struct ethernetFrame *, unsigned int);
extern void udpGPI_replyToSender(void);


#endif // __FIRMATA_GPI_OVER_UDP_H_

