
void parseFrame(char*, unsigned short);



struct sensorPacket {
    char dest[6];
    char src[6];
    char prot[2];

// This is the payload, 42 to 1500 bytes:
    char subProt[4];
    uint16_t adcVal;
    uint16_t xAccel0;
    uint16_t yAccel0;
    uint16_t zAccel0;
    uint16_t xAccel1;
    uint16_t yAccel1;
    uint16_t zAccel1;
    char happyMessage[24];

    uint32_t fcs;
};

