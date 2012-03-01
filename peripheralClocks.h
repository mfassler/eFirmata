/*
 * Copyright 2012, Mark Fassler
 *
 */

#ifndef __PERIPHERAL_CLOCKS_H
#define __PERIPHERAL_CLOCKS_H

#define PCLK_WDT        0
#define PCLK_TIMER0     1
#define PCLK_TIMER1     2
#define PCLK_UART0      3
#define PCLK_UART1      4
#define PCLK_ADC        10
#define PCLK_UART2      21

uint32_t getPeripheralClock (int);

#endif // __PERIPHERAL_CLOCKS_H
