/**
 *   Name: cs8900.c
 *     Ver.: 1.0
 *     Date: 07/05/2001
 *     Auth: Andreas Dannenberg
 *     HTWK Leipzig
 *     university of applied sciences
 *     Germany
 *  Func: ethernet packet-driver for use with LAN-
 *  controller CS8900 from Crystal/Cirrus Logic
 *
 *  Keil: Module modified for use with Philips
 *  LPC17xx EMAC Ethernet controller
 *
 */

/**
 * Copyright 2012, 2013 Mark Fassler
 *  Licensed under the GPLv3
 *
 */


// TODO:  proper CRCs.  See:  http://www.edaboard.com/thread120700.html


#include "LPC17xx.h"
#include <type.h>
#include "bitTypes.h"

#include "ethernetPHY.h"
#include "emac.h"
#include "network/ethernet.h"

#include "debug.h"
#include "timer.h"


// Tell the EMAC where to receive data into DMA memory:
void rxDescriptorInit (void) {
	unsigned int i;

	// The order of the RX buffers never changes, so we set
	// those descriptors here only once:
	for (i = 0; i < NUM_RX_FRAG; i++) {
		RX_DESC_PACKET(i)  = RX_BUF(i);
		RX_DESC_CTRL(i)	= RCTRL_INT | (ETH_FRAG_SIZE-1);
		RX_STAT_INFO(i)	= 0;
		RX_STAT_HASHCRC(i) = 0;
	}

	// How do we find our incoming buffers?
	LPC_EMAC->RxDescriptor	= RX_DESC_BASE;
	LPC_EMAC->RxStatus		= RX_STAT_BASE;
	LPC_EMAC->RxDescriptorNumber = NUM_RX_FRAG-1;

	// Starting off at 0:
	LPC_EMAC->RxConsumeIndex  = 0;
}


// Tell the EMAC where to transmit data from DMA memory:
void txDescriptorInit(void) {
	// The order of the TX buffers might change (depending on what data
	// we happen to be sending) so those descriptors are set on-the-fly
	// as we send data; we do not set those here.

	// How do we find our outgoing buffers?
	LPC_EMAC->TxDescriptor	= TX_DESC_BASE;
	LPC_EMAC->TxStatus		= TX_STAT_BASE;
	LPC_EMAC->TxDescriptorNumber = NUM_TX_FRAG-1;

	// Starting off at 0:
	LPC_EMAC->TxProduceIndex  = 0;
}


void setLinkMode(uint8_t speed, uint8_t duplex) {
	// This tells the Ethernet controller (on-board the micro-controller, 
	// not the PHY) what mode to go into.

	if (speed) {  // 100 Mbit
		debug("100 Mbit");
		LPC_EMAC->SUPP = SUPP_SPEED;

	} else { // 10 Mbit
		debug("10 Mbit");
		LPC_EMAC->SUPP = 0;
	}

	if (duplex) {  // Full-duplex
		debug("		Full-duplex");
		LPC_EMAC->MAC2	|= MAC2_FULL_DUP;
		LPC_EMAC->Command |= CR_FULL_DUP;
		LPC_EMAC->IPGT	 = IPGT_FULL_DUP;

	} else {   // Half-duplex
		debug("		Half-duplex");
		LPC_EMAC->IPGT = IPGT_HALF_DUP;
	}
}


void Init_EMAC(void) {

	// Power Up the EMAC controller.
	LPC_SC->PCONP |= (0x1<<30);

	LPC_GPIO1->FIOPIN = bit18; // Blinky LED #1

	LPC_PINCON->PINSEL2 = 0x50150105;
	LPC_PINCON->PINSEL3 &= ~0x0000000F;
	LPC_PINCON->PINSEL3 |= 0x00000005;

	// Reset all EMAC internal modules.
	LPC_EMAC->MAC1 = MAC1_RES_TX | MAC1_RES_MCS_TX | MAC1_RES_RX | MAC1_RES_MCS_RX |
			  MAC1_SIM_RES | MAC1_SOFT_RES;
	LPC_EMAC->Command = CR_REG_RES | CR_TX_RES | CR_RX_RES;
	delayMs(0, 100);

	LPC_GPIO1->FIOPIN = bit20; // Blinky LED #2

	// Initialize MAC control registers.
	LPC_EMAC->MAC1 = MAC1_PASS_ALL;
	LPC_EMAC->MAC2 = MAC2_CRC_EN | MAC2_PAD_EN;
	LPC_EMAC->MAXF = ETH_MAX_FLEN;
	LPC_EMAC->CLRT = CLRT_DEF;
	LPC_EMAC->IPGR = IPGR_DEF;

	// Enable Reduced MII interface.
	LPC_EMAC->Command = CR_RMII | CR_PASS_RUNT_FRM;

	// Reset Reduced MII Logic.
	LPC_EMAC->MCFG = MCFG_CLK_DIV20 | MCFG_RES_MII;
	delayMs(0, 100);
	LPC_EMAC->MCFG = MCFG_CLK_DIV20;

	enetPHY_Init();  // blinks LED#3, ends on LED#4

	// Set the Ethernet MAC Address registers
	// myMacAddress is declared in main.c
	LPC_EMAC->SA0 = (myMacAddress[5] << 8) | myMacAddress[4];
	LPC_EMAC->SA1 = (myMacAddress[3] << 8) | myMacAddress[2];
	LPC_EMAC->SA2 = (myMacAddress[1] << 8) | myMacAddress[0];

	// Initialize Tx and Rx DMA Descriptors
	rxDescriptorInit();
	txDescriptorInit();

	// Receive Broadcast and Perfect Match Packets
	LPC_EMAC->RxFilterCtrl = RFC_BCAST_EN | RFC_PERFECT_EN;

	// Enable EMAC interrupts.
	LPC_EMAC->IntEnable = INT_RX_DONE | INT_TX_DONE;

	// Reset all interrupts
	LPC_EMAC->IntClear  = 0xFFFF;

	// Enable receive and transmit mode of MAC Ethernet core
	LPC_EMAC->Command  |= (CR_RX_EN | CR_TX_EN);
	LPC_EMAC->MAC1	 |= MAC1_REC_EN;

	LPC_GPIO1->FIOPIN = bit18; // Blinky LED #1
}



void ENET_IRQHandler (void) {
	unsigned int idx;

	// On receive, the EMAC "produces" buffers.
	// We must "consume" those buffers.  
	while(LPC_EMAC->RxConsumeIndex != LPC_EMAC->RxProduceIndex) {

		// The buffer that we must consume:
		idx = LPC_EMAC->RxConsumeIndex;

		parseFrame( (struct ethernetFrame *)RX_DESC_PACKET(idx),  // The address of the buffer
					(RX_STAT_INFO(idx) & RINFO_SIZE) - 3  // The length of the buffer
				  );

		// Buffer consumed.  Move on to the next buffer:
		idx++;
		if (idx == NUM_RX_FRAG) {
			idx = 0;
		}

		LPC_EMAC->RxConsumeIndex = idx;
	}

	// No more buffers to consume; clear the interrupt:
	LPC_EMAC->IntClear  = 0xFFFF;
}



void ethernetPleaseSend(void * bufAddr, unsigned short frameSize) {
	// For DMA, we must "Produce" buffers.  The eth-DMA controller will
	// "Consume" those buffers
	unsigned int idx;

	// What is the next buffer index?
	idx = LPC_EMAC->TxProduceIndex;

	TX_STAT_INFO(idx) = 0;

	// The literal memory address is fed to the EMAC-DMA controller:
	TX_DESC_PACKET(idx) = (unsigned int) bufAddr;
	TX_DESC_CTRL(idx) = (frameSize - 1) | TCTRL_LAST;

	// EMAC, you have work do do:
	idx++;
	if (idx == NUM_TX_FRAG) {
		idx = 0;
	}
	LPC_EMAC->TxProduceIndex = idx;
}

