
/***************************************************************************
 *
 * The mbed and the LPCXpresso both use different PHYs for their ethernet.
 * The PHYs are very similar, but with slight differences.
 *
 ***************************************************************************/


#include "LPC17xx.h"
#include <type.h>

#include "ethernetPHY.h"
#include "emac.h"

#include "debug.h"
#include "timer.h"


void enetPHY_write (unsigned int PhyReg, unsigned short Value)
{
	/* Write a data 'Value' to PHY register 'PhyReg'. */
	uint32_t tout;

	/* Hardware MII Management for LPC176x devices. */
	LPC_EMAC->MADR = DP83848C_DEF_ADR | PhyReg;
	LPC_EMAC->MWTD = Value;

	/* Wait utill operation completed */
	for (tout = 0; tout < MII_WR_TOUT; tout++)
	{
		if ((LPC_EMAC->MIND & MIND_BUSY) == 0)
		{
			break;
		}
	}
	if (tout == MII_WR_TOUT)
		debug("enetPHY_write timeout");
	debugWord("enetPHY_write() tout:", tout);
}



unsigned short enetPHY_read (unsigned int PhyReg)
{
	/* Read a PHY register 'PhyReg'. */
	unsigned int tout, val;

	LPC_EMAC->MADR = DP83848C_DEF_ADR | PhyReg;
	LPC_EMAC->MCMD = MCMD_READ;

	/* Wait until operation completed */
	for (tout = 0; tout < MII_RD_TOUT; tout++)
	{
		if ((LPC_EMAC->MIND & MIND_BUSY) == 0)
		{
			break;
		}
	}
	if (tout == MII_RD_TOUT)
		debug("enetPHY_read timeout");
	LPC_EMAC->MCMD = 0;
	val = LPC_EMAC->MRDD;
	return val;
}


void enetPHY_reset()
{
	uint32_t tout;
	unsigned int regv;
	// Put the PHY into reset mode:  
	// (Both the DP83848 and the 8720 use this register)
	debug("Resetting enet PHY...");
	enetPHY_write (PHY_REG_BMCR, 0x8000);

	// Wait for the reset to end.
	for (tout = 0; tout < 0x100000; tout++)
	{
		// The dp83848 docs say that the bit will stay "one" until
		// the reset is finished.  The 8720 docs don't say such a 
		// thing and this bit seems to clear instantly with the 8720.
		// The 8720 docs say that reset will be complete within 0.5s.
		regv = enetPHY_read (PHY_REG_BMCR);
		if (!(regv & 0x8000))  // Reset complete
		{
			debugLong("Reset complete. tout: ", tout);
			break;
		}
	}
	if (tout < 5)
	{
		// Reset happened suspiciously fast.  Wait for 0.5s
		delayMs(0, 500);
	}
	debugWord("PHY_REG_BMCR: ", regv);
}



void enetPHY_AutoNegotiate()
{
	uint32_t tout;
	unsigned int regv;
	// both PHYs auto-negotiate in the same way, so we make
	// one function for now.

	// Tell the PHY to auto-negotiate link speed:
	enetPHY_write (PHY_REG_BMCR, PHY_AUTO_NEG);

	// Wait to complete
	for (tout = 0; tout < 0x100000; tout++)
	{
		regv = enetPHY_read (PHY_REG_BMSR);
		if (regv & 0x0020) // Autonegotiation Complete. 
		{
			debugWord("PHY_REG_BMSR: ", regv);
			debug("auto-neg complete");
			break;
		}
		if ((tout & 0x00004000))
		{
			LPC_GPIO1->FIOPIN = 0x00;  // Blinky lights off
		}
		else
		{
			LPC_GPIO1->FIOPIN = (1<<21); // Blinky LED #3 on
		}
	}
	if (tout == 0x100000)
		debug("PHY auto-negotiation timout");

	// Typical tout on mbed is about 80000.  LPCXpresso is about 65000
	//debugLong("PHY_AUTO_NEG tout: ", tout);

	LPC_GPIO1->FIOPIN = (1<<23); // Blinky LED #1
}



void enetPHY_Init_DP83848()
{
	//uint32_t tout;
	unsigned int regv;
	unsigned int speed = 0;
	unsigned int duplex = 0;

	enetPHY_AutoNegotiate();

	regv = enetPHY_read (PHY_REG_STS);

	duplex = (regv & 0x0004) ? 1 : 0;
	speed = (regv & 0x0002) ? 0 : 1;

	setLinkMode(speed, duplex);
}



void enetPHY_Init_LAN8720()
{
	//uint32_t tout;
	unsigned int regv;
	unsigned int hcdSpeed = 0;

	enetPHY_AutoNegotiate();

	regv = enetPHY_read(31);

	hcdSpeed = (regv & 0x001c) >> 2;
	switch (hcdSpeed)
	{
		case 1: // 10 Mbit, Half-duplex
		  setLinkMode(0, 0);
		  break;
		case 5: // 10 Mbit, Full-duplex
		  setLinkMode(0, 1);
		  break;
		case 2: // 100 Mbit, Half-duplex
		  setLinkMode(1, 0);
		  break;
		case 6: // 100 Mbit, Full-dupex
		  setLinkMode(1, 1);
		  break;
		default:
		  debug("PHY_LAN8720 error: Unknown link rate.  Assuming 100 Mbit, full-duplex.");
		  setLinkMode(1, 1);
		  break;
	}
}



void enetPHY_Init()
{
	unsigned int id1, id2;

	enetPHY_reset();

	// What PHY do we have?
	// (Both the DP83848 and the 8720 use these registers)
	id1 = enetPHY_read(PHY_REG_IDR1);
	id2 = enetPHY_read(PHY_REG_IDR2);
	debugWord("id1: ", id1);
	debugWord("id2: ", id2);

	if (((id1 << 16) | (id2 & 0xFFF0)) == DP83848C_ID)
	{
		// This is used on the mbed
		debug("Found PHY: DP83848C");
		enetPHY_Init_DP83848();
	}
	else if (((id1 << 16) | (id2 & 0xFFF0)) == LAN8720_ID)
	{
		// This is used on the LPCXpresso
		debug("Found PHY: LAN8720");
		enetPHY_Init_LAN8720();
	}
}



