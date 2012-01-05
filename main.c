
#include <LPC17xx.h>
#include "uart.h"
#include "EMAC.h"

volatile uint32_t current_time;

const unsigned char myAddress[6] = {0x00, 0x02, 0xf7, 0xf0, 0xff, 0xee};
const unsigned char aiuAddress[6] = {0xE0, 0xCB, 0x4E, 0x47, 0x7F, 0x9B};
const unsigned char myProtocol[2] = {0x08, 0x1c};

//char * prepareEthernetFrame()
//{
    char etherFrame[] = {
        0xE0, 0xCB, 0x4E, 0x47, 0x7F, 0x9B,   //dest addr
        0x00, 0x02, 0xf7, 0xaa, 0xbb, 0xcc,   //my address
        0x08, 0x1c, //my fake protocol
        // 14 bytes here
        97, 98, 99, 32, 97, 98, 99, 32, // "abc abc "
        97, 98, 99, 32, 97, 98, 99, 32, // "abc abc "
        97, 98, 99, 32, 97, 98, 113, 32, // "abc abq "
        97, 98, 99, 32, 97, 98, 99, 32, // "abc abc "
        97, 98, 99, 32, 97, 98, 99, 32, // "abc abc "
        97, 98, 122, 32, 97, 98, 100, 32 // "abz abd "
        // 48 more bytes, 48+14 = 62
    };


//    memcpy(&oneFrame[0], &aiuAddress, 6);
//    memcpy(&oneFrame[6], &myAddress, 6);
//    return oneFrame;
//}



void SysTick_Handler (void)
{
    current_time++;
}

void delay (uint32_t interval)
{
    uint32_t start = current_time;
    while (current_time - start < interval);
}


const char hexDigits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

int formatHex(char * buffer, uint8_t value)
{
    buffer[0] = hexDigits[(value & 0xf0) >> 4];
    buffer[1] = hexDigits[(value & 0x0f)];
    return 2;
}

int formatHexWord(char * buffer, uint16_t value)
{
    buffer[0] = hexDigits[(value & 0xf000) >> 12];
    buffer[1] = hexDigits[(value & 0x0f00) >> 8];
    buffer[2] = hexDigits[(value & 0x00f0) >> 4];
    buffer[3] = hexDigits[(value & 0x000f)];
    return 4;
}

void mySerSend(char *buf, uint32_t Length)
{
    while (Length != 0)
    {
        while(! (LPC_UART1->LSR) ); // wait for the TX fifo to clear
        LPC_UART1->THR = *buf;
        buf++;
        Length--;
        if (*buf == 0x00) 
        {
            break;
        }
    }
}

void debugByte(char* msg, uint8_t value)
{
    char buf[2];
    int bufLen;
    mySerSend(msg, 200);
    mySerSend("0x", 2);
    bufLen = formatHex(buf, value);
    mySerSend(buf, bufLen);
    mySerSend("\n\r", 2);
}

void debugWord(char* msg, uint16_t value)
{
    char buf[4];
    int bufLen;
    mySerSend(msg, 200);
    mySerSend("0x", 2);
    bufLen = formatHexWord(buf, value);
    mySerSend(buf, bufLen);
    mySerSend("\n\r", 2);
}

int main() {
    char *bufA = "start\n\r";
    char *bufB = "stop\n\r";
    char *bufC = " ..init done\n\r";
//    unsigned char fbuffer[256];
//    int fBufLen;
//    char *etherFrame;

    SysTick_Config (SystemCoreClock / 1000);


    LPC_GPIO1->FIODIR = (1 << 18) | (1 << 20) | (1 << 21) | (1 << 23);

    LPC_GPIO1->FIOSET = (1 << 18);
    delay(120);
    LPC_GPIO1->FIOCLR = (1 << 18);
    delay(120);
    LPC_GPIO1->FIOSET = (1 << 18);
    delay(120);
    LPC_GPIO1->FIOCLR = (1 << 18);

    UARTInit(1, 38400);
    mySerSend(bufC, 14);

    Init_EMAC();
    delay(1000);
    mySerSend("Done with Init_EMAC().\r\n", 200);

//    etherFrame = prepareEthernetFrame();
    RequestSend(62);
    CopyToFrame_EMAC( (unsigned short *) etherFrame, 62);



    debugByte("This is a number: ", 34);

    delay (2000);
    while (1)
    {
        etherFrame[60]++;
        if (etherFrame[60] == 122)
        {
            etherFrame[60] = 97;
        }
        RequestSend(62);
        CopyToFrame_EMAC( (unsigned short *) etherFrame, 62);


        delay(120);
        LPC_GPIO1->FIOSET = (1 << 20) | (1<<23);
        mySerSend(bufA, 7);

        delay(120);
        LPC_GPIO1->FIOCLR = (1 << 20) | (1<<23);
        mySerSend(bufB, 6);

//        fBufLen = sprintf(fbuffer, "Hello! This is a number: %02x\n\r", 254);
//        mySerSend(bufB, 6);
//        mySerSend(fbuffer, fBufLen);
//        UARTSend(0, buffer, 1);
//        LPC_UART1->THR = 0x42;
//        UART1TxEmpty = 0;
    
    }
}

