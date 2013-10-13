/*
 *  Copyright 2012, Mark Fassler
 *  Licensed under the GPLv3
 *
 */


#include <LPC17xx.h>
#include <type.h>
#include "bitTypes.h"

#include "peripheralClocks.h"

#include "adc.h"
#include "debug.h"

#include "emac.h"
#include "network/ethernet.h"
#include "network/ip.h"
#include "network/udp.h"
#include "network/udpServices/oscope.h"
#include "network/endian.h"


extern struct ethernetFrame *bigEtherFrameA;
extern struct ethernetFrame *bigEtherFrameB;

volatile uint8_t triggerLevel;
volatile uint8_t triggerDirection;
volatile uint8_t triggerChannel;
volatile uint8_t triggerEnabled;
volatile uint16_t triggerNumSamplesReq;
volatile uint8_t adc_weAreSending;

volatile uint8_t currentChannel;


void ADC_startOneSample(void) {
	uint8_t clkdiv = 1;
	uint8_t whichChannelBitmask;
	uint32_t startMode;
	startMode = bit24; // for the ADC->CR, "START" conversion now

	whichChannelBitmask = 1 << (currentChannel & 0x7);

	//LPC_ADC->ADINTEN = bit1 | bit2 | bit4 | bit5; // Enable interrupts for channels 1, 2, 4 and 5
	LPC_ADC->ADINTEN = bit8; // we only care about a single, global interrupt for ADC conversion

	LPC_ADC->ADCR = whichChannelBitmask
		| ( clkdiv << 8 )
		| bit21 // PDN = 1, normal operation
		| startMode;
}


void ADC_IRQHandler(void) {

	uint8_t adcValue;

	//regVal = LPC_ADC->ADSTAT;  // Read ADC will clear the interrupt?

	switch (currentChannel) {
		case 1:
			adcValue = (LPC_ADC->ADDR1 & 0xfff0) >> 8;
			currentChannel = 2;
			ADC_startOneSample();
			adc2network(1, adcValue);
			break;
		case 2:
			adcValue = (LPC_ADC->ADDR2 & 0xfff0) >> 8;
			currentChannel = 4;
			ADC_startOneSample();
			adc2network(2, adcValue);
			break;
		case 4:
			adcValue = (LPC_ADC->ADDR4 & 0xfff0) >> 8;
			currentChannel = 5;
			ADC_startOneSample();
			adc2network(4, adcValue);
			break;
		case 5:
			adcValue = (LPC_ADC->ADDR5 & 0xfff0) >> 8;
			currentChannel = 1;
			ADC_startOneSample();
			adc2network(5, adcValue);
			break;
		default:
			debug("adc: wtf?");
	}

	//if (tmp & bit30) {
	//	debug("adc overrun");
	//}
}



void adc2network(uint8_t adcChannel, uint8_t adcValue) {
	struct ethernetFrame *aFrame;
	struct ipPacket *ip;
	struct udpPacket *udp;
	struct scopeData *tod;

	static uint8_t whichFrame =0;
	static uint16_t whichSampleInPayload = 0;
	static uint16_t adc_currentSampleNumber = 0;
	uint16_t payloadIdx;
	unsigned int payloadSize;

	uint8_t offset;
	static uint8_t prevTriggerSamples[] = {0,0,0};

	switch (adcChannel) {
		case 1:
			offset = 0;
			break;
		case 2:
			offset = 1;
			break;
		case 4:
			offset = 2;
			break;
		case 5:
			offset = 3;
			break;
		default:
			offset = 0;
	}

	if (adc_weAreSending) {

		// We have two ethernet frames in DMA memory.  We alternate between
		// those two frames:
		if (whichFrame == 0) {
			aFrame = bigEtherFrameA;
		} else {
			aFrame = bigEtherFrameB;
		}
		ip = (struct ipPacket*) &aFrame->payload;
		udp = (struct udpPacket*) &ip->data;
		tod = (struct scopeData*) &udp->data;

		tod->bytesPerSample = 4;

		// Write a byte to the next position in the frame:
		payloadIdx = whichSampleInPayload * tod->bytesPerSample + offset;
		tod->data[payloadIdx] = adcValue;

		if (adcChannel > 4) {
			whichSampleInPayload++;
			adc_currentSampleNumber++;
		}

		// When we fill up a buffer (a eFrame/IP/UDP/eFirmata packet) or if we've
		// hit the max number of samples, we'll send a packet
		if ((adcChannel > 4) && ( (payloadIdx > 1231) || (adc_currentSampleNumber == triggerNumSamplesReq) )) {
			tod->totalSamples = htons(whichSampleInPayload);
			//tod->totalSamples = htons(256);
			tod->startSampleNumber = htonl(adc_currentSampleNumber - whichSampleInPayload);

			// Ethernet header is 14 bytes.
			// IP header is 20 bytes.
			// UDP header is 8 bytes.
			// TOD header is 24 bytes.
			//payloadSize = tod->bytesPerSample * tod->totalSamples;  // ha.  totalSamples is big-endian
			payloadSize = tod->bytesPerSample * whichSampleInPayload;
			whichSampleInPayload = 0;

			ip->totalLength = htons(20 + 8 + 20 + payloadSize);
			udp->length = htons(8 + 20 + payloadSize);

			if (whichFrame == 0) {
				whichFrame = 1;
			} else {
				whichFrame = 0;
			}

			ip->headerChecksum = 0;
			ip->headerChecksum = internetChecksum(ip, 20);

			ethernetPleaseSend(aFrame, (14 + 20 + 8 + 20 + payloadSize));
		}

		// When we hit the max number of samples, we stop:
		if (adc_currentSampleNumber > triggerNumSamplesReq) {
			adc_weAreSending = 0;
			triggerEnabled = 0;
			adc_currentSampleNumber = 0;
			whichSampleInPayload = 0;
		}

	} else { // waiting for a trigger

		if ((adcChannel == triggerChannel) && triggerEnabled) {

			if (triggerDirection) { // 1 is rising, 0 is falling
				if ((prevTriggerSamples[2] < triggerLevel) && (prevTriggerSamples[1] < triggerLevel) &&
					(prevTriggerSamples[0] > triggerLevel) && (adcValue > triggerLevel)) {
					adc_weAreSending = 1;
					triggerEnabled = 0;
				}

			} else { // falling trigger

				if ((prevTriggerSamples[2] > triggerLevel) && (prevTriggerSamples[1] > triggerLevel) &&
					(prevTriggerSamples[0] < triggerLevel) && (adcValue < triggerLevel)) {
					adc_weAreSending = 1;
					triggerEnabled = 0;
				}

			}
			prevTriggerSamples[2] = prevTriggerSamples[1];
			prevTriggerSamples[1] = prevTriggerSamples[0];
			prevTriggerSamples[0] = adcValue;
		}
	}

	return;
}


void ADCInit(void) {
	uint32_t pclk;
	//uint8_t clkdiv;

	LPC_SC->PCONP |= bit12; // Power Control PCADC bit

	adc_weAreSending = 1;

	// We're going to use four ADC pins: 
	//	P0.24 == AD0.1 (pin 16 on the mbed)
	//	P0.25 == AD0.2 (pin 17 on the mbed)
	//	P1.30 == AD0.4 (pin 19 on the mbed)
	//	P1.31 == AD0.5 (pin 20 on the mbed)

	LPC_PINCON->PINSEL1 &= ~0x000f0000;  // AD0.1 and AD0.2
	LPC_PINCON->PINSEL1 |= 0x00050000;  // AD0.1 and AD0.2

	LPC_PINCON->PINSEL3 |= 0xf0000000;  // AD0.4 and AD0.5

	// No pull-up no pull-down (function 10) on these ADC pins:
	LPC_PINCON->PINMODE1 &= ~0x000f0000;  // AD0.1 and AD0.2
	LPC_PINCON->PINMODE1 |= 0x000a0000;  // AD0.1 and AD0.2
	LPC_PINCON->PINMODE3 &= ~0xf0000000;  // AD0.4 and AD0.5
	LPC_PINCON->PINMODE3 |= 0xa0000000;  // AD0.4 and AD0.5


	// The ADC clock rate is:   pclk / (clkdiv + 1)
	// We're aiming for slightly less than 13 MHz.
	// It takes 65 clocks for one sample, so the maximum
	// sample rate is 13MHz / 65 clocks = 200000 SPS.
	// There's only one real ADC, but the inputs are multiplexed,
	// so the sample rate is divided amongst each input.  
	pclk = getPeripheralClock(PCLK_ADC);
	debugLong("ADC's pclk: ", pclk);
	// ** Example:
	//  clkdiv = (pclk / SAMPLE_RATE - 1)

/*
	// In theory, this should give me a sample rate of 192.3 KSPS, but in fact
	// it gives about 195.2 KSPS (~1.5% higher).  Not sure why...
	//clkdiv = 1;
	clkdiv = 3;

	LPC_ADC->ADINTEN = bit1 | bit2 | bit4 | bit5; // Enable interrupts for channels 1, 2, 4 and 5

	LPC_ADC->ADCR = bit1 | bit2 | bit4 | bit5 // Enable channels 1, 2, 4 and 5
		| ( clkdiv << 8 )
		| bit16  // BURST
		| bit21;  // PDN = 1, normal operation
		//| ( 0 << 24 )   // START = 0 A/D conversion stops
		//| ( 0 << 27 );  // EDGE = 0 (CAP/MAT singal falling,trigger A/D conversion)
*/

	NVIC_EnableIRQ(ADC_IRQn);

	currentChannel = 1; // Global var, used by ADC_startOneSample():
	ADC_startOneSample();
}

