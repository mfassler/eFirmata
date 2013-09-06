
#ifndef __OSCOPE_OVER_UDP_H_
#define __OSCOPE_OVER_UDP_H_


#define TRIGGERMODE_OFF 0
#define TRIGGERMODE_NOW 1
#define TRIGGERMODE_RISING 2
#define TRIGGERMODE_FALLING 3
#define TRIGGERMODE_CONTINUOUS 4

struct scopeCmdOverUdp {
	char idToken[8]; // For *incoming udp* must always be: "eFirmata" (so that we ignore random crap)
	char idSubToken[3]; // For incoming UDP, must always be:  "TOS" for Triggered Oscillo-Scope.
	uint8_t version; // Protocol version.  Right now, only 0x00 is allowed. 
	uint32_t extra; // This will be for flags, options, etc.  Ignored for now.  Set to zeros.
	uint8_t triggerMode;
	uint8_t triggerChannel;
	uint8_t triggerLevel;
	uint8_t nothing; // stay on 32-bit boundaries
	uint32_t triggerNumSamplesReq;
};

extern void incomingOscopeOverUdp(struct ethernetFrame *, unsigned int);
extern void prepOutgoingFastBuffers(struct ethernetFrame *, unsigned int);

#endif // __OSCOPE_OVER_UDP_H_

