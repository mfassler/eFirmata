/****************************************************************************
 *   $Id:: ssp.h 5741 2010-11-30 23:13:47Z usb00423                         $
 *   Project: NXP LPC17xx SSP example
 *
 *   Description:
 *     This file contains SSP code header definition.
 *
 ****************************************************************************
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * products. This software is supplied "AS IS" without any warranties.
 * NXP Semiconductors assumes no responsibility or liability for the
 * use of the software, conveys no license or title under any patent,
 * copyright, or mask work right to the product. NXP Semiconductors
 * reserves the right to make changes in the software without
 * notification. NXP Semiconductors also make no representation or
 * warranty that such application will be suitable for the specified
 * use without further testing or modification.
****************************************************************************/
#ifndef __SSP_H__
#define __SSP_H__

#define TX_RX_ONLY		0		/* 1 is TX or RX only depending on SSP_SLAVE
								flag, 0 is either loopback mode or communicate
								with a serial EEPROM. */

/* SPI read and write buffer size */
#define SSP_BUFSIZE		16
#define FIFOSIZE		8

#define DELAY_COUNT		10
#define MAX_TIMEOUT		0xFF

/* Port0.2 is the SSP select pin */
#define SSP0_SEL		(1 << 2)
	
/* SSP Status register */
#define SSPSR_TFE		(1 << 0)
#define SSPSR_TNF		(1 << 1) 
#define SSPSR_RNE		(1 << 2)
#define SSPSR_RFF		(1 << 3) 
#define SSPSR_BSY		(1 << 4)

/* SSP CR0 register */
#define SSPCR0_DSS		(1 << 0)
#define SSPCR0_FRF		(1 << 4)
#define SSPCR0_SPO		(1 << 6)
#define SSPCR0_SPH		(1 << 7)
#define SSPCR0_SCR		(1 << 8)

/* SSP CR1 register */
#define SSPCR1_LBM		(1 << 0)
#define SSPCR1_SSE		(1 << 1)
#define SSPCR1_MS		(1 << 2)
#define SSPCR1_SOD		(1 << 3)

/* SSP Interrupt Mask Set/Clear register */
#define SSPIMSC_RORIM	(1 << 0)
#define SSPIMSC_RTIM	(1 << 1)
#define SSPIMSC_RXIM	(1 << 2)
#define SSPIMSC_TXIM	(1 << 3)

/* SSP0 Interrupt Status register */
#define SSPRIS_RORRIS	(1 << 0)
#define SSPRIS_RTRIS	(1 << 1)
#define SSPRIS_RXRIS	(1 << 2)
#define SSPRIS_TXRIS	(1 << 3)

/* SSP0 Masked Interrupt register */
/*
#define SSPMIS_RORMIS	(1 << 0)
#define SSPMIS_RTMIS	(1 << 1)
#define SSPMIS_RXMIS	(1 << 2)
#define SSPMIS_TXMIS	(1 << 3)
*/

/* SSP0 Interrupt clear register */
/*
#define SSPICR_RORIC	(1 << 0)
#define SSPICR_RTIC		(1 << 1)
*/

/* ATMEL SEEPROM command set */
#define WREN		0x06		/* MSB A8 is set to 0, simplifying test */
#define WRDI		0x04
#define RDSR		0x05
#define WRSR		0x01
#define READ		0x03
#define WRITE		0x02

/* RDSR status bit definition */
#define RDSR_RDY	0x01
#define RDSR_WEN	0x02

/* If RX_INTERRUPT is enabled, the SSP RX will be handled in the ISR
SSPReceive() will not be needed. */
extern void SSP0_IRQHandler (void);
extern void SSP1_IRQHandler (void);
extern void SSP0Init( void );
extern void SSP1Init( void );
extern int SSP0Send(char *Buf, int Length);
extern void SSP0_sendBytes(void);
extern void SSP1Send(uint8_t *Buf, uint32_t Length);
extern void SSPReceive( uint32_t portnum, uint8_t *buf, uint32_t Length );

extern void getAccel(uint8_t, uint16_t*, uint16_t*, uint16_t*);

#endif  // __SSP_H__

