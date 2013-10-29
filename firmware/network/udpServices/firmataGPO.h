
#ifndef __FIRMATA_GPO_OVER_UDP_H_
#define __FIRMATA_GPO_OVER_UDP_H_

struct gpoMaskPins {
	uint32_t mask;
	uint32_t pins;
};

struct gpoCmdOverUdp {
	char idToken[8]; // For *incoming udp* must always be: "eFirmata" (so that we ignore random crap)
	char idSubToken[3]; // For incoming UDP, must always be:  "GPO"
	uint8_t version; // Protocol version.  Right now, only 0x00 is allowed. 
	struct gpoMaskPins maskPins[5];
};


extern void initGPO(void);
extern void udpGPO(struct ethernetFrame *, unsigned int);


#endif // __FIRMATA_GPO_OVER_UDP_H_

