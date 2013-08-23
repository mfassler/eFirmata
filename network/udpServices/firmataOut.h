
#ifndef __FIRMATA_OVER_UDP_
#define __FIRMATA_OVER_UDP_


// We only have 6 PWMs on this board, but the protocol can support ~1000 8bit PWMs, if
// they'll fit into a single UDP packet..

// 8bit PWMs are grouped into batches of 32 (so that the mask will align onto a 32bit boundary)
struct pwmBatch {
	uint8_t mask[4]; // Bitmask.  If it's a 1, then we set the PWM to the new value.  0==ignore.
	uint8_t values[32];
};

struct pwmCmdOverUdp {
	char idToken[8]; // For *incoming udp* must always be: "eFirmata" (so that we ignore random crap)
	char idSubToken[3]; // For incoming UDP, must always be:  "PWM"
	uint8_t version; // Protocol version.  Right now, only 0x00 is allowed. 
	uint32_t extra; // This will be for flags, options, etc.  Ignored for now.  Set to zeros.
	struct pwmBatch batch[3];
};



void udpPWM(struct ethernetFrame *, unsigned int);


#endif // __FIRMATA_OVER_UDP_


