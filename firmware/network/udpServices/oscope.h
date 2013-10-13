
#ifndef __OSCOPE_OVER_UDP_H_
#define __OSCOPE_OVER_UDP_H_


// These are the possible commands that a client (workstation) can send to us:
#define TRIGGERMODE_OFF 0
#define TRIGGERMODE_NOW 1
#define TRIGGERMODE_RISING 2
#define TRIGGERMODE_FALLING 3
#define TRIGGERMODE_CONTINUOUS 4 // Not implemented yet


struct scopeCmdOverUdp {
	char idToken[8]; // For *incoming udp* must always be: "eFirmata" (so that we ignore random crap)
	char idSubToken[3]; // For incoming UDP, must always be:  "TOC" for Triggered Oscilloscope Command.
	uint8_t version; // Protocol version.  Right now, only 0x00 is allowed. 
	uint32_t extra; // This will be for flags, options, etc.  Ignored for now.  Set to zeros.
	uint8_t triggerMode;
	uint8_t triggerChannel;
	char triggerDatatype;
	uint8_t nothing; // stay on 32-bit boundaries
	uint32_t triggerLevel;
	uint32_t triggerNumSamplesReq;
};



#define TWO_POINT_LINEAR 1

struct channelMetaData {

	// From physics.nist.gov:  "V" is for volts, "A" is for amps
	//  (in the future, we might also want "dollars to yen", "radians", "dollars per barrel", etc...)
	char units;

	// For datatypes, we use Python structs:   http://docs.python.org/2/library/struct.html
	//   Over the network, we always use "Network Order"  (big endian)

	//  ** Regular INTEGER types:
	//  'b' -- signed char, 1 byte
	//  'B' -- unsigned char, 1 byte
	//  'h' -- short, 2 bytes
	//  'H' -- unsigned short, 2 bytes
	//  'i' -- int, 4 bytes
	//  'I' -- unsigned int, 4 bytes
	//  'q' -- long long, 8 bytes
	//  'Q' -- unsigned long long, 8 bytes

	//  ** FLOAT types:
	//  'f' -- float, 4 bytes
	//  'd' -- double, 8 bytes

	//  (Python also defines 'c', for a string of length one, which we do not use here)

	char dataDataType;
	char realDataType;
	char scaleType;  // enum.  right now, there's just "two-point linear"

	char errorType; // enum.  Right now, there's just 0 for "none".   (and all of my experimental physics professors gasp in horror...)

	char nothing[3]; // padding to end up on a 32-bit boundary


	// Two points define a line.  
	//  "dataVal" is the number in the UDP packet
	//  "realVal" is the real measurement of something physical

	uint32_t dataValA;
	char realValA[8]; // hard-code enough space to hold a double
	uint32_t dataValB;
	char realValB[8]; // hard-code enough space to hold a double

	// This is reserved for future use (when I actually define error):
	char errorParameters[32];
};


struct scopeMetaData {
	char idToken[8]; // For *outgoing udp* must always be:  "JimWinks"
	char idSubToken[3]; // Must always be "TOM", for Triggered Oscilloscope Metadata
	uint8_t version;  // protocol version 0

	// The domain (the time axis) must always be a simple, linear scale.  The step-size must be
	// the same every sample.  (eg:  "every sample is 1 ms apart")
	// From physics.nist.gov, "s" is for "seconds", "V" is for "volts"
	char domainPhysicalUnit;
	char stepSizeDatatype;  // 'f' for float, 'd' for double
	uint8_t numChannels;
	uint8_t bytesPerDescriptor;

	//  FIXME.  Hrmm.... this is an INT-only chip...   but we can hardcode a float... hrmm...
	//double domainStepSize; // 64-bit float.  
	char domainStepSize[8]; // Tmp.  Fixme.  change me.

	struct channelMetaData channels[];
};


struct scopeData {
	char idToken[8]; // For *outgoing udp* must always be:  "JimWinks"
	char idSubToken[3]; // Must always be "TOD", for Triggered Oscilloscope Data
	uint8_t version;  // protocol version 0

	uint8_t bytesPerSample;
	char nothing;
	uint16_t totalSamples; // in this packet

	uint32_t startSampleNumber;  // Of this packet.  Count from zero.

	char data[];  // bytesPerSample * totalSamples = the length of this data
};


extern void incomingOscopeOverUdp(struct ethernetFrame *, unsigned int);

extern void oscope_sendMetaData(uint32_t, char *, uint16_t, uint16_t);
extern void prepOutgoingFastBuffers(uint32_t, char *, uint16_t, uint16_t);
//extern void prepOutgoingFastBuffers(struct ethernetFrame *, unsigned int);

#endif // __OSCOPE_OVER_UDP_H_

