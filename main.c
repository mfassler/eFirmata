
#include <LPC17xx.h>
#include "uart.h"
#include "debug.h"
#include "EMAC.h"
#include "adc.h"
#include "dac.h"
#include "pwm.h"

#include "MAC_ADDRESSES.h"
const char myAddress[] = SELF_ADDR;
const char destAddress[] = DEST_ADDR;


extern volatile uint16_t ADCValue[ADC_NUM];
extern volatile uint32_t ADCIntDone;
extern volatile uint32_t OverRunCounter;


volatile uint32_t current_time;

//const unsigned char myAddress[6] = {0x00, 0x02, 0xf7, 0xf0, 0xff, 0xee};
//const unsigned char aiuAddress[6] = {0xE0, 0xCB, 0x4E, 0x47, 0x7F, 0x9B};
const unsigned char myProtocol[2] = {0x08, 0x1c};

//char * prepareEthernetFrame()
//{
    char etherFrame[] = {
        0xE0, 0xCB, 0x4E, 0x47, 0x7F, 0x9B,   //dest addr
        0x00, 0x02, 0xf7, 0xaa, 0xbb, 0xcc,   //my address
        0x08, 0x1c, //my fake protocol
        // 14 bytes here
        97, 98, 99, 32, 48, 120, 99, 32, // "abc 0xc "
        97, 98, 32, 32, 97, 98, 99, 32, // "ab  abc "
        97, 98, 99, 32, 97, 98, 113, 32, // "abc abq "
        97, 98, 99, 32, 97, 98, 99, 32, // "abc abc "
        97, 98, 99, 32, 97, 98, 99, 32, // "abc abc "
        97, 98, 122, 32, 97, 98, 100, 32, 32 // "abz abd  "
        // 49 more bytes, 49+14 = 63
    };


//    memcpy(&oneFrame[0], &aiuAddress, 6);
//    memcpy(&oneFrame[6], &myAddress, 6);
//    return oneFrame;
//}

char bigEtherFrame[2][6+6+2+1024+4];


void SysTick_Handler (void)
{
    current_time++;
}

void delay (uint32_t interval)
{
    uint32_t start = current_time;
    while (current_time - start < interval);
}


/// WTF does this do??  Does anything care about this?
extern volatile uint32_t match_counter1;


int main() {
    char adcText[4];
    int i;

    LPC_GPIO1->FIODIR = (1 << 18) | (1 << 20) | (1 << 21) | (1 << 23);
    LPC_GPIO2->FIODIR = 0x00003fc0; // p2.6 through p2.13

    LPC_GPIO1->FIOSET = (1 << 18);

    SysTick_Config (SystemCoreClock / 1000);

    UARTInit(1, 38400);

    debug("...init done.");

    LPC_GPIO1->FIOSET = (1 << 20);

//    UARTInit(0, 38400);

    // Initialize Ethernet:
    Init_EMAC();
    NVIC_EnableIRQ(ENET_IRQn);
    LPC_GPIO1->FIOSET = (1 << 21);
    debug("Done with Init_EMAC().");

    // Set our addresses: 
    for (i=0; i<6; i++)
    {
        etherFrame[i]     = destAddress[i];
        bigEtherFrame[0][i] = destAddress[i];
        bigEtherFrame[1][i] = destAddress[i];
        etherFrame[i+6]     = myAddress[i];
        bigEtherFrame[0][i+6] = myAddress[i];
        bigEtherFrame[1][i+6] = myAddress[i];
    }
    bigEtherFrame[0][12] = 0x08;
    bigEtherFrame[1][12] = 0x08;
    bigEtherFrame[0][13] = 0x1d;
    bigEtherFrame[1][13] = 0x1d;

    // Send one ethernet frame:
    RequestSend(62);
    CopyToFrame_EMAC( (unsigned short *) etherFrame, 62);
    LPC_GPIO1->FIOSET = (1 << 23);


    ADCInit(ADC_CLK);
    DACInit();
    PWM_Init();
//    ADCRead(5);

//    PWM_Set(1, 256, 0);
    PWM_Start();
    i = 0;
    while (1)
    {
        etherFrame[60]++;
        if (etherFrame[60] == 122)
        {
            etherFrame[60] = 97;
        }

        formatHexWord(adcText, ADCValue[5]);
        etherFrame[20] = adcText[0];
        etherFrame[21] = adcText[1];
        etherFrame[22] = adcText[2];
        etherFrame[23] = adcText[3];

        RequestSend(62);
        CopyToFrame_EMAC( (unsigned short *) etherFrame, 62);

        delay(120);
        LPC_GPIO1->FIOSET = (1 << 20) | (1<<23);

        delay(120);
        LPC_GPIO1->FIOCLR = (1 << 20) | (1<<23);

        i++;

        switch (i)
        {
            case 1:
              LPC_DAC->DACR = (0x00 << 6);
/*              LPC_PWM1->MR1 = 600;
              LPC_PWM1->MR2 = 600;
              LPC_PWM1->MR3 = 600;
              LPC_PWM1->LER = LER0_EN | LER1_EN | LER2_EN | LER3_EN | LER4_EN | LER5_EN | LER6_EN;*/
//              PWM_Set(1, 1200, 0);
              break;
            case 5:
              LPC_DAC->DACR = (0x3ff << 6);
/*              LPC_PWM1->MR1 = 1000;
              LPC_PWM1->MR2 = 1000;
              LPC_PWM1->MR3 = 1000;
              LPC_PWM1->LER = LER0_EN | LER1_EN | LER2_EN | LER3_EN | LER4_EN | LER5_EN | LER6_EN;*/
              break;
            case 10:
              LPC_DAC->DACR = (0x1ff << 6);
              break;
            case 15:
              i=0;
              break;
        }
    }
}

