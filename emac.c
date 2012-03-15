/******************************************************************
 *****                                                        *****
 *****  Name: cs8900.c                                        *****
 *****  Ver.: 1.0                                             *****
 *****  Date: 07/05/2001                                      *****
 *****  Auth: Andreas Dannenberg                              *****
 *****        HTWK Leipzig                                    *****
 *****        university of applied sciences                  *****
 *****        Germany                                         *****
 *****  Func: ethernet packet-driver for use with LAN-        *****
 *****        controller CS8900 from Crystal/Cirrus Logic     *****
 *****                                                        *****
 *****  Keil: Module modified for use with Philips            *****
 *****        LPC17xx EMAC Ethernet controller                *****
 *****                                                        *****
 ******************************************************************/


// TODO:  proper CRCs.  See:  http://www.edaboard.com/thread120700.html


#include "LPC17xx.h"
#include <type.h>

#include "emac.h"
#include "debug.h"
#include "firmataProtocol.h"

#include "MAC_ADDRESSES.h"

#include "timer.h"




static void write_PHY (unsigned int PhyReg, unsigned short Value)
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
        debug("write_PHY timeout");
    debugWord("write_PHY() tout:", tout);
}


static unsigned short read_PHY (unsigned int PhyReg)
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
        debug("read_PHY timeout");
    LPC_EMAC->MCMD = 0;
    val = LPC_EMAC->MRDD;
    return val;
}

// Tell the EMAC where to receive data into DMA memory:
void rxDescriptorInit (void)
{
    unsigned int i;

    // The order of the RX buffers never changes, so we set
    // those descriptors here only once:
    for (i = 0; i < NUM_RX_FRAG; i++)
    {
        RX_DESC_PACKET(i)  = RX_BUF(i);
        RX_DESC_CTRL(i)    = RCTRL_INT | (ETH_FRAG_SIZE-1);
        RX_STAT_INFO(i)    = 0;
        RX_STAT_HASHCRC(i) = 0;
    }

    // How do we find our incoming buffers?
    LPC_EMAC->RxDescriptor    = RX_DESC_BASE;
    LPC_EMAC->RxStatus        = RX_STAT_BASE;
    LPC_EMAC->RxDescriptorNumber = NUM_RX_FRAG-1;

    // Starting off at 0:
    LPC_EMAC->RxConsumeIndex  = 0;
}


// Tell the EMAC where to transmit data from DMA memory:
void txDescriptorInit(void)
{
    // The order of the TX buffers might change (depending on what data
    // we happen to be sending) so those descriptors are set on-the-fly
    // as we send data; we do not set those here.

    // How do we find our outgoing buffers?
    LPC_EMAC->TxDescriptor    = TX_DESC_BASE;
    LPC_EMAC->TxStatus        = TX_STAT_BASE;
    LPC_EMAC->TxDescriptorNumber = NUM_TX_FRAG-1;

    // Starting off at 0:
    LPC_EMAC->TxProduceIndex  = 0;
}



void Init_EMAC(void)
{
    unsigned int regv, id1, id2;
    uint32_t tout;
    const char myAddress[] = SELF_ADDR;

    LPC_GPIO1->FIOPIN = (1<<18); // Blinky LED #1

    // Power Up the EMAC controller.
    LPC_SC->PCONP |= (0x1<<30);

  
    LPC_PINCON->PINSEL2 = 0x50150105;
    LPC_PINCON->PINSEL3 &= ~0x0000000F;
    LPC_PINCON->PINSEL3 |= 0x00000005;

    /* Reset all EMAC internal modules. */
    LPC_EMAC->MAC1 = MAC1_RES_TX | MAC1_RES_MCS_TX | MAC1_RES_RX | MAC1_RES_MCS_RX |
              MAC1_SIM_RES | MAC1_SOFT_RES;
    LPC_EMAC->Command = CR_REG_RES | CR_TX_RES | CR_RX_RES;
    delayMs(0, 100);

    LPC_GPIO1->FIOPIN = (1<<20); // Blinky LED #2

    /* Initialize MAC control registers. */
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

    /* Put the DP83848C in reset mode */
    write_PHY (PHY_REG_BMCR, 0x8000);

    /* Wait for hardware reset to end. */
    for (tout = 0; tout < 0x100000; tout++) 
    {
        regv = read_PHY (PHY_REG_BMCR);
        if (!(regv & 0x8000))  // Reset complete
        {
            debug("Reset complete");
            break;
        }
    }
    debugWord("PHY_REG_BMCR: ", regv);

    LPC_GPIO1->FIOPIN = (1<<21); // Blinky LED #3

    /* Check if this is a DP83848C PHY. */
    id1 = read_PHY (PHY_REG_IDR1);
    debugWord("id1: ", id1);
    id2 = read_PHY (PHY_REG_IDR2);
    debugWord("id2: ", id2);
    //if (((id1 << 16) | (id2 & 0xFFF0)) == DP83848C_ID) 
    //if (((id1 << 16) | (id2 & 0xFFF0)) == LAN8720_ID) 
    if ( 0 ) 
    {
        //debug("This is a DP-83848");
        debug("This is a LAN-8720.  Doing auto-negotiation:...");
        /* Configure the PHY device */

        /* Use autonegotiation about the link speed. */
        write_PHY (PHY_REG_BMCR, PHY_AUTO_NEG);

        /* Wait to complete Auto_Negotiation. */
        for (tout = 0; tout < 0x100000; tout++)
        {
            regv = read_PHY (PHY_REG_BMSR);
            if (regv & 0x0020) // Autonegotiation Complete. 
            {
                debugWord("PHY_REG_BMSR: ", regv);
                debug("auto-neg complete");
                break;
            }
            if ((tout & 0x00004000))
            {
                LPC_GPIO1->FIOPIN = 0x00;
            }
            else
            {
                LPC_GPIO1->FIOPIN = (1<<21); // Blinky LED #3
            }
        }
        if (tout == 0x100000)
            debug("auto-neg timout");
        debugLong("PHY_AUTO_NEG tout: ", tout);
    }

    LPC_GPIO1->FIOPIN = (1<<23); // Blinky LED #4

    /* Check the link status. */
    for (tout = 0; tout < 0x10000; tout++)
    {
        regv = read_PHY (PHY_REG_STS);
        if (regv & 0x0001) // Link is on
        {
            debugWord("PHY_REG_STS: ", regv);
            debug("got link");
            break;
        }
        if ((tout & 0x00004000))
        {
            LPC_GPIO1->FIOPIN = 0x00;
        }
        else
        {
            LPC_GPIO1->FIOPIN = (1<<23); // Blinky LED #4
        }
    }

    LPC_GPIO1->FIOPIN = (1<<18); // Blinky LED #1

    if (regv & 0x0004) // Full duplex is enabled. 
    {
        LPC_EMAC->MAC2    |= MAC2_FULL_DUP;
        LPC_EMAC->Command |= CR_FULL_DUP;
        LPC_EMAC->IPGT     = IPGT_FULL_DUP;
    }
    else // Half duplex
    {
      LPC_EMAC->IPGT = IPGT_HALF_DUP;
    }

    if (regv & 0x0002) // 10MBit 
    {
        LPC_EMAC->SUPP = 0;
        debug("10 mbit");
    }
    else // 100MBit
    {
        LPC_EMAC->SUPP = SUPP_SPEED;
        debug("100 mbit");
    }

    LPC_GPIO1->FIOPIN = (1<<20); // Blinky LED #2

    /* Set the Ethernet MAC Address registers */
    LPC_EMAC->SA0 = (myAddress[5] << 8) | myAddress[4];
    LPC_EMAC->SA1 = (myAddress[3] << 8) | myAddress[2];
    LPC_EMAC->SA2 = (myAddress[1] << 8) | myAddress[0];

    /* Initialize Tx and Rx DMA Descriptors */
    rxDescriptorInit();
    txDescriptorInit();
    LPC_GPIO1->FIOPIN = (1<<21); // Blinky LED #3

    /* Receive Broadcast and Perfect Match Packets */
    LPC_EMAC->RxFilterCtrl = RFC_BCAST_EN | RFC_PERFECT_EN;

    /* Enable EMAC interrupts. */
    LPC_EMAC->IntEnable = INT_RX_DONE | INT_TX_DONE;

    /* Reset all interrupts */
    LPC_EMAC->IntClear  = 0xFFFF;

    LPC_GPIO1->FIOPIN = (1<<23); // Blinky LED #4

    /* Enable receive and transmit mode of MAC Ethernet core */
    LPC_EMAC->Command  |= (CR_RX_EN | CR_TX_EN);
    LPC_EMAC->MAC1     |= MAC1_REC_EN;
}



void ENET_IRQHandler (void)
{
    unsigned int idx;

    // On receive, the EMAC "produces" buffers.
    // We must "consume" those buffers.  
    while(LPC_EMAC->RxConsumeIndex != LPC_EMAC->RxProduceIndex)
    {
        // The buffer that we must consume:
        idx = LPC_EMAC->RxConsumeIndex;

        parseFrame( (char *)RX_DESC_PACKET(idx),           // The address of the buffer
                    (RX_STAT_INFO(idx) & RINFO_SIZE) - 3   // The length of the buffer
                  );

        // Buffer consumed.  Move on to the next buffer:
        idx++;
        if (idx == NUM_RX_FRAG)
            idx = 0;

        LPC_EMAC->RxConsumeIndex = idx;
    }

    // No more buffers to consume; clear the interrupt:
    LPC_EMAC->IntClear  = 0xFFFF;
}



void ethernetPleaseSend(unsigned short whichBuffer, unsigned short frameSize)
{
    unsigned int idx;

    // What is the next buffer index?
    idx = LPC_EMAC->TxProduceIndex + 1;
    if (idx == NUM_TX_FRAG)
        idx = 0;

    // Point the way to the buffer:
    TX_DESC_CTRL(idx)   = 0;
    TX_STAT_INFO(idx)   = 0;
    TX_DESC_PACKET(idx) = TX_BUF(whichBuffer);
    TX_DESC_CTRL(idx) = frameSize | TCTRL_LAST;

    // EMAC, you have work do do:
    LPC_EMAC->TxProduceIndex = idx;
}

