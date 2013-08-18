/*
 *  Copyright 2012, Mark Fassler
 *  Licensed under the GPLv3
 *
 */


#ifndef __ADC_H 
#define __ADC_H

#define ADC_OFFSET          0x10
#define ADC_INDEX           4

#define ADC_DONE            0x80000000
#define ADC_OVERRUN         0x40000000
#define ADC_ADINT           0x00010000

#define ADC_NUM			8		/* for LPCxxxx */



extern volatile uint8_t triggerLevel;
extern volatile uint8_t triggerDirection;
extern volatile uint8_t triggerChannel;
extern volatile uint8_t triggerEnabled;
extern volatile uint16_t triggerNumSamplesReq;
extern volatile uint8_t adc_weAreSending;



extern void ADC_IRQHandler(void);
extern void ADCInit(void);
extern void adc2network(uint8_t, uint8_t );


#endif // __ADC_H
