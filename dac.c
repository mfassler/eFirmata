
#include "LPC17xx.h"
#include "type.h"
#include "dac.h"

void DACInit( void )
{
	/* setup the related pin to DAC output */
	LPC_PINCON->PINSEL1 = 0x00200000; /* set p0.26 to DAC output */

	LPC_DAC->DACCNTVAL = 0x00FF;
	LPC_DAC->DACCTRL = (0x1<<1)|(0x1<<2);
	return;
}

