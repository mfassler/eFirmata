
#include <LPC17xx.h>
#include "uart.h"
#include "debug.h"
#include "emac.h"
#include "ssp.h"
#include "adc.h"
#include "dac.h"
#include "pwm.h"

extern volatile uint16_t ADCValue[ADC_NUM];
extern volatile uint32_t ADCIntDone;
extern volatile uint32_t OverRunCounter;

#include "MAC_ADDRESSES.h"



char etherFrame[] = {
    // 14 bytes here:
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   //dest addr
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   //src addr
    0x00, 0x00, //protocol

    // 49 more bytes:
    97, 98, 99, 32, 48, 120, 99, 32, // "abc 0xc "
    97, 98, 32, 32, 97, 98, 99, 32, // "ab  abc "
    97, 98, 99, 32, 97, 98, 113, 32, // "abc abq "
    97, 98, 99, 32, 97, 98, 99, 32, // "abc abc "
    97, 98, 99, 32, 97, 98, 99, 32, // "abc abc "
    97, 98, 122, 32, 97, 98, 100, 32, 32 // "abz abd  "

    // 49 + 14 = 63 total
};

char bigEtherFrame[2][6+6+2+1024+4];


void setAddressesAndProtocol(void)
{
    const char myAddress[] = SELF_ADDR;
    const char destAddress[] = DEST_ADDR;
    const char protocol[2] = EFIRMATA_PROTOCOL;
    const char fastProtocol[2] = EFIRMATA_PROTOCOL_FAST;

    int i;

    for (i=0; i<6; i++)
    {
        etherFrame[i]     = destAddress[i];
        bigEtherFrame[0][i] = destAddress[i];
        bigEtherFrame[1][i] = destAddress[i];
        etherFrame[i+6]     = myAddress[i];
        bigEtherFrame[0][i+6] = myAddress[i];
        bigEtherFrame[1][i+6] = myAddress[i];
    }
    etherFrame[12] = protocol[0];
    bigEtherFrame[0][12] = fastProtocol[0];
    bigEtherFrame[1][12] = fastProtocol[0];
    etherFrame[13] = protocol[1];
    bigEtherFrame[0][13] = fastProtocol[1];
    bigEtherFrame[1][13] = fastProtocol[1];
}




volatile uint32_t current_time;
volatile int doJiffyAction = 0;
void jiffyAction (void);

void SysTick_Handler (void)
{
    current_time++;
    if (doJiffyAction)
        jiffyAction();
}

void delay (uint32_t interval)
{
    uint32_t start = current_time;
    while (current_time - start < interval);
}

void jiffyAction (void)
{
    // Once every 10 ms we send sensor data to the PC.
    char xAccelText[4];
    char yAccelText[4];
    char zAccelText[4];
    uint16_t xAccel;
    uint16_t yAccel;
    uint16_t zAccel;
    char adcText[4];

    // For now, we'll only run once every 16 jiffies:
    if ((current_time & 0xf) != 0)
        return;

    formatHexWord(adcText, ADCValue[5]);
    etherFrame[20] = adcText[0];
    etherFrame[21] = adcText[1];
    etherFrame[22] = adcText[2];
    etherFrame[23] = adcText[3];

    getAccel(0, &xAccel, &yAccel, &zAccel);
    formatHexWord(xAccelText, xAccel);
    formatHexWord(yAccelText, yAccel);
    formatHexWord(zAccelText, zAccel);

    etherFrame[26] = xAccelText[0];
    etherFrame[27] = xAccelText[1];
    etherFrame[28] = xAccelText[2];
    etherFrame[29] = xAccelText[3];
    etherFrame[30] = 32;

    etherFrame[31] = yAccelText[0];
    etherFrame[32] = yAccelText[1];
    etherFrame[33] = yAccelText[2];
    etherFrame[34] = yAccelText[3];
    etherFrame[35] = 32;

    etherFrame[36] = zAccelText[0];
    etherFrame[37] = zAccelText[1];
    etherFrame[38] = zAccelText[2];
    etherFrame[39] = zAccelText[3];
    etherFrame[40] = 32;

    RequestSend(62);
    CopyToFrame_EMAC( (unsigned short *) etherFrame, 62);
}



int main() {
    // Enable our blinky LEDs:
    LPC_GPIO1->FIODIR = (1 << 18) | (1 << 20) | (1 << 21) | (1 << 23);

    LPC_GPIO1->FIOSET = (1 << 18);  // Turn blinky LED #1

    UARTInit(2, 38400);
    debug("...init done.");

    LPC_GPIO1->FIOSET = (1 << 20);  // Turn blinky LED #2

    // Start the 100 Hz timer:
    SysTick_Config (SystemCoreClock / 100);

    // Initialize Ethernet:
    Init_EMAC();
    NVIC_EnableIRQ(ENET_IRQn);

    LPC_GPIO1->FIOSET = (1 << 21);  // Turn on blinky LED #3
    debug("Done with Init_EMAC().");

    // Set our addresses: 
    setAddressesAndProtocol();

    // Send one ethernet frame:
    RequestSend(62);
    CopyToFrame_EMAC( (unsigned short *) etherFrame, 62);

    LPC_GPIO1->FIOSET = (1 << 23);  // Turn on blinky LED #4



    // ***** BEGIN:  Initialize peripherials

    // Disable UART0 and UART1 for power-saving:
    //LPC_SC->PCONP &= ~((1 << 3) | (1 << 4));

    // Enable some digital outputs:
    //LPC_GPIO2->FIODIR = 0x00003fc0; // P2.6 through P2.13

    ADCInit();
    DACInit();
    PWM_Init();
    PWM_Start();
    SSP1Init();

    // ***** END:  Initialize peripherials


    doJiffyAction = 1;

    while (1)
    {
        etherFrame[60]++;
        if (etherFrame[60] == 122)
        {
            etherFrame[60] = 97;
        }

        delay(12);
        LPC_GPIO1->FIOSET = (1 << 20) | (1<<23);  // heartbeat

        delay(12);
        LPC_GPIO1->FIOCLR = (1 << 20) | (1<<23);  // heartbeat
    }
}

