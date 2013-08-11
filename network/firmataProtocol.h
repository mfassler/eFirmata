
void parseFrame(char*, unsigned short);
void initOutgoingEthernetPackets(void);


struct sensorPacket {
	char dest[6];
	char src[6];
	char prot[2];

// This is the payload, 42 to 1500 bytes:
	char subProt[4];

	uint8_t inputByte;

	uint16_t stepperPosition;
	uint16_t targetPosition;
	uint16_t busyBit;
	//uint16_t adcVal;
	//uint16_t xAccel0;
	//uint16_t yAccel0;
	//uint16_t zAccel0;
	//uint16_t xAccel1;
	//uint16_t yAccel1;
	//uint16_t zAccel1;
	int16_t quadPositionA;
	char happyMessage[24];

	uint32_t fcs;
};

struct incomingFirmataPacket {
	uint8_t inputByte;
	uint8_t nothing1;
	uint8_t nothing2;
	uint8_t pwm1;
	uint8_t pwm2;
	uint8_t pwm3;
	uint8_t pwm4;
	uint8_t pwm5;
	uint8_t pwm6;
	uint8_t numSspBytes;
	char sspCommand[15];
	char stepperCmd;
	uint16_t setPosition;
};

#define TRIGGERMODE_OFF 0
#define TRIGGERMODE_ONESHOT 1
#define TRIGGERMODE_RISING 2
#define TRIGGERMODE_FALLING 3
#define TRIGGERMODE_CONTINUOUS 4

struct incomingFirmataControlPacket {
	uint8_t triggerMode;
	uint8_t triggerChannel;
	uint8_t triggerLevel;
	uint8_t triggerNumFramesReq;
};


struct udpPacket {
	uint8_t enetDest[6];
	uint8_t enetSrc[6];
	uint8_t enetProtocol[2]; // IP packet inside Ethernet

	// This is the ethernet payload, 42 to 1500 bytes:
	uint8_t ipVersion;  // 4bit version, 4bit header length
	uint8_t diffServicesField; // 6bit DSCP and 3bit ECN
	uint16_t totalLength;
	uint16_t identification;
	uint16_t flagsAndFragOffset; // 3bit flags, 13bit fragment offset
	uint8_t ttl;
	uint8_t ipProtocol; // UDP packet inside IP
	uint16_t ipHeaderChecksum;
	uint8_t srcIpAddr[4];
	uint8_t destIpAddr[4];
	uint16_t srcPort;
	uint16_t destPort;
	uint16_t length;
	uint16_t udpChecksum;
	unsigned char data[5];

	// Back to the ethernet frame:
	uint32_t fcs;
};



struct bigEtherFrame {
	char dest[6];
	char src[6];
	char prot[2];

	uint8_t data[1030];

	uint32_t fcs;
};
