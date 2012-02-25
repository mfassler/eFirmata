#include <type.h>

int formatHex(char* , uint8_t);
int formatHexWord(char*, uint16_t);
int formatHexLong(char*, uint32_t);
void UART1_sendString(char*, uint8_t);
void UART2_sendString(char*, uint8_t);
void debug(char*);
void debugByte(char*, uint8_t);
void debugWord(char*, uint16_t);
void debugLong(char*, uint32_t);


