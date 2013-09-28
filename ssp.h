
#ifndef __SSP_H__
#define __SSP_H__


#define FIFOSIZE 8

extern void SSP0_IRQHandler(void);
extern void SSP0Init(void);

void SSP0_tryToSend(void);

extern void SSP0_receiveIntoFIFO(char *, uint8_t);
extern void SSP0_pleaseSend(char *, uint8_t);



#endif  // __SSP_H__


