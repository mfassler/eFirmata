/*
 * Copyright 2012, Mark Fassler
 *
 */

#include <LPC17xx.h>
#include <type.h>
#include "peripheralClocks.h"

#include "timer.h"


void delayMs(uint8_t whichTimer, uint32_t delayInMs)
{
    uint32_t pclk;

    if (whichTimer == 0)
    {
        pclk = getPeripheralClock(PCLK_TIMER0);
        LPC_TIM0->TCR = 0x02;           // reset timer
        LPC_TIM0->PR  = pclk / 1000;    // 1000 counts per second
        LPC_TIM0->MR0 = delayInMs;
        LPC_TIM0->IR  = 0xff;           // reset all interrrupts
        LPC_TIM0->MCR = 0x04;           // stop when MR0 matches TC
        LPC_TIM0->TCR = 0x01;           // start timer

        // wait:
        while (LPC_TIM0->TCR & 0x01);
    }
    else if (whichTimer == 1)
    {
        pclk = getPeripheralClock(PCLK_TIMER1);
        LPC_TIM1->TCR = 0x02;           // reset timer
        LPC_TIM1->PR  = pclk / 1000;    // 1000 counts per second
        LPC_TIM1->MR0 = delayInMs;
        LPC_TIM1->IR  = 0xff;           // reset all interrrupts
        LPC_TIM1->MCR = 0x04;           // stop when MR0 matches TC
        LPC_TIM1->TCR = 0x01;           // start timer

        // wait:
        while (LPC_TIM1->TCR & 0x01);
    }
    return;
}

