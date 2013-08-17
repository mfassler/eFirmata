
struct udpPacket {
	uint16_t srcPort;
	uint16_t destPort;
	uint16_t length;
	uint16_t udpChecksum;
	unsigned char data[5];
};


