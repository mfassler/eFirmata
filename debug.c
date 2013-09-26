/*
 *  Copyright 2012, Mark Fassler
 *  Licensed under the GPLv3
 *
 */


#include <LPC17xx.h>
#include "debug.h"
#include "uart.h"


const char hexDigits[] = {'0', '1', '2', '3', 
						  '4', '5', '6', '7', 
						  '8', '9', 'a', 'b', 
						  'c', 'd', 'e', 'f'};


int formatHex(char * buffer, uint8_t value) {
	buffer[0] = hexDigits[(value & 0xf0) >> 4];
	buffer[1] = hexDigits[(value & 0x0f)];
	return 2;
}


int formatHexWord(char * buffer, uint16_t value) {

	buffer[0] = hexDigits[(value & 0xf000) >> 12];
	buffer[1] = hexDigits[(value & 0x0f00) >> 8];
	buffer[2] = hexDigits[(value & 0x00f0) >> 4];
	buffer[3] = hexDigits[(value & 0x000f)];
	return 4;
}


int formatHexLong(char * buffer, uint32_t value) {

	buffer[0] = hexDigits[(value & 0xf0000000) >> 28];
	buffer[1] = hexDigits[(value & 0x0f000000) >> 24];
	buffer[2] = hexDigits[(value & 0x00f00000) >> 20];
	buffer[3] = hexDigits[(value & 0x000f0000) >> 16];
	buffer[4] = hexDigits[(value & 0x0000f000) >> 12];
	buffer[5] = hexDigits[(value & 0x00000f00) >> 8];
	buffer[6] = hexDigits[(value & 0x000000f0) >> 4];
	buffer[7] = hexDigits[(value & 0x0000000f)];
	return 8;
}


void UART1_sendString(char *buf, uint8_t length) {

	while (length != 0) {
		while(! (LPC_UART1->LSR) ); // wait for the TX fifo to clear
		LPC_UART1->THR = *buf;
		buf++;
		length--;

		if (*buf == 0x00) {
			break;
		}
	}
}


void UART2_sendString(char *buf, uint8_t length) {

	while (length != 0) {
		while(! (LPC_UART2->LSR) ); // wait for the TX fifo to clear
		LPC_UART2->THR = *buf;
		buf++;
		length--;

		if (*buf == 0x00) {
			break;
		}
	}
}


void debug(char *msg) {

	UART2_sendString(msg, 200);
	UART2_sendString("\r\n", 2);
}


void debugByte(char* msg, uint8_t value) {

	char buf[2];
	int bufLen;

	UART2_sendString(msg, 200);
	UART2_sendString("0x", 2);
	bufLen = formatHex(buf, value);
	UART2_sendString(buf, bufLen);
	UART2_sendString("\r\n", 2);
}


void debugWord(char* msg, uint16_t value) {

	char buf[4];
	int bufLen;
	UART2_sendString(msg, 200);
	UART2_sendString("0x", 2);
	bufLen = formatHexWord(buf, value);
	UART2_sendString(buf, bufLen);
	UART2_sendString("\r\n", 2);
}


void debugLong(char* msg, uint32_t value) {

	char buf[8];
	int bufLen;
	UART2_sendString(msg, 200);
	UART2_sendString("0x", 2);
	bufLen = formatHexLong(buf, value);
	UART2_sendString(buf, bufLen);
	UART2_sendString("\r\n", 2);
}


void debugMacAddress(char* msg, char* macAddress) {
	int i;
	char buf[2];
	int bufLen;

	UART2_sendString(msg, 200);

	for (i=0; i<6; i++) {
		bufLen = formatHex(buf, macAddress[i]);
		UART2_sendString(buf, bufLen);
		if (i != 5) {
			UART2_sendString(":", 1);
		}
	}
	UART2_sendString("\r\n", 2);
}

