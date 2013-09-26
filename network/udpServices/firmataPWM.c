
#include <LPC17xx.h>
#include "bitTypes.h"

#include "debug.h"

#include "network/endian.h"
#include "network/ethernet.h"
#include "network/ip.h"
#include "network/udp.h"

#include "network/udpServices/firmataPWM.h"

//#include "pwm.h"


void udpPWM(struct ethernetFrame *frame, unsigned int length) {
	const char FIRMATA_ID_SUBTOKEN[3] = "PWM"; // This is PWM
	const uint8_t FIRMATA_PWM_VERSION = 0;

	uint32_t pwmMask;

	struct ipPacket *ip;
	struct udpPacket *udp;
	struct pwmCmdOverUdp *cmd;
	unsigned short i;

	ip = (struct ipPacket*) &frame->payload;
	udp = (struct udpPacket*) &ip->data;
	cmd = (struct pwmCmdOverUdp*) &udp->data;

	// Check that this packet starts with the string "eFirmata"
	for (i=0; i<8; i++) {
		if (cmd->idToken[i] != FIRMATA_ID_TOKEN[i]) {
			return;
		}
	}

	// Check that this is for PWM:
	for (i=0; i<3; i++) {
		if (cmd->idSubToken[i] != FIRMATA_ID_SUBTOKEN[i]) {
			return;
		}
	}

	// We only support protocol version 0:
	if (cmd->version != FIRMATA_PWM_VERSION) {
		return;
	}

	pwmMask = ntohl(cmd->batch[0].mask);

	// On the LPC1768/9, the PWM #0 is not available as an output,
	// so we start counting from 1, right along with our microcontroller
	if (pwmMask & bit1)
		LPC_PWM1->MR1 = cmd->batch[0].values[1];

	if (pwmMask & bit2)
		LPC_PWM1->MR2 = cmd->batch[0].values[2];

	if (pwmMask & bit3)
		LPC_PWM1->MR3 = cmd->batch[0].values[3];

	if (pwmMask & bit4)
		LPC_PWM1->MR4 = cmd->batch[0].values[4];

	if (pwmMask & bit5)
		LPC_PWM1->MR5 = cmd->batch[0].values[5];

	if (pwmMask & bit6)
		LPC_PWM1->MR6 = cmd->batch[0].values[6];

	LPC_PWM1->LER = pwmMask & 0x7e;  // Use our mask to set the output latches
}



