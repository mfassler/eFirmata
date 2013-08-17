
#ifndef __FIRMATA_PROTOCOL_H
#define __FIRMATA_PROTOCOL_H


struct sensorPacket {
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
#define TRIGGERMODE_NOW 1
#define TRIGGERMODE_RISING 2
#define TRIGGERMODE_FALLING 3
#define TRIGGERMODE_CONTINUOUS 4

struct incomingFirmataControlPacket {
	uint8_t triggerMode;
	uint8_t triggerChannel;
	uint8_t triggerLevel;
	uint8_t triggerNumFramesReq;
};


extern void parseIncomingFirmataControlPacket(struct incomingFirmataControlPacket *);
extern void parseIncomingFirmataPacket(struct incomingFirmataPacket *);

#endif // __FIRMATA_PROTOCOL_H

