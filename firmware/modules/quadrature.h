
#ifndef __QUADRATURE_H
#define __QUADRATURE_H

extern void EINT3_IRQHandler(void);
void Quadrature_Init(void);
void quadrature_updatePosition(void);
int16_t quadrature_getPosition(uint8_t);

#endif // __QUADRATURE_H

