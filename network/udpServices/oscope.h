
#ifndef __OSCOPE_OVER_UDP_
#define __OSCOPE_OVER_UDP_


#define TRIGGERMODE_OFF 0
#define TRIGGERMODE_NOW 1
#define TRIGGERMODE_RISING 2
#define TRIGGERMODE_FALLING 3
#define TRIGGERMODE_CONTINUOUS 4

struct scopeCmdOverUdp {
	char idToken[8]; // For *incoming udp* must always be: "eFirmata" (so that we ignore random crap)
	uint32_t extra; // This will be for flags, options, protoVersion, etc.  Ignored for now.
	uint8_t triggerMode;
	uint8_t triggerChannel;
	uint8_t triggerLevel;
	uint8_t triggerNumFramesReq;
};

extern void incomingOscopeOverUdp(struct ethernetFrame *, unsigned int);
extern void prepOutgoingFastBuffers(struct ethernetFrame *, unsigned int);

#endif // __OSCOPE_OVER_UDP_

