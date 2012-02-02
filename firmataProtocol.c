
#include <LPC17xx.h>
#include "debug.h"
#include "pwm.h"

void parseFrame(char* input, unsigned short inputLen)
{
    debugByte("protocol0: ", input[12]);
    debugByte("protocol1: ", input[13]);
    if ((input[12] == 0x08) && (input[13] == 0x1c))  // etherType == eFirmata
    {
        if ( input[14] == 83 )   // 83 = "S"
        {
            LPC_GPIO2->FIOSET = (input[15] << 6);
        }
        else if ( input[14] == 67 )  // 67 = "C"
        {
            LPC_GPIO2->FIOCLR = (input[15] << 6);
        }

        LPC_PWM1->MR1 = input[17];
        LPC_PWM1->MR2 = input[18];
        LPC_PWM1->MR3 = input[19];
        LPC_PWM1->MR4 = input[20];
        LPC_PWM1->MR5 = input[21];
        LPC_PWM1->MR6 = input[22];
        LPC_PWM1->LER = LER0_EN | LER1_EN | LER2_EN | LER3_EN | LER4_EN | LER5_EN | LER6_EN;
    }
}
