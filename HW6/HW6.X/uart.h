#include<xc.h>           // processor SFR definitions
#define PIC32_SYS_FREQ 48000000
#define PIC32_DESIRED_BAUD 230400

void initUart();
void PIC32_WriteUART1(const char * string);