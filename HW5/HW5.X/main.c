#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include<math.h>
#include<limits.h>
// DEVCFG0
#pragma config DEBUG = OFF // disable debugging
#pragma config JTAGEN = OFF // disable jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF // disable flash write protect
#pragma config BWP = OFF // disable boot write protect
#pragma config CP = OFF // disable code protect

// DEVCFG1
#pragma config FNOSC = FRCPLL // use fast frc oscillator with pll
#pragma config FSOSCEN = OFF // disable secondary oscillator
#pragma config IESO = OFF // disable switching clocks
#pragma config POSCMOD = OFF // primary osc disabled
#pragma config OSCIOFNC = OFF // disable clock output
#pragma config FPBDIV = DIV_1 // divide sysclk freq by 1 for peripheral bus clock
#pragma config FCKSM = CSDCMD // disable clock switch and FSCM
#pragma config WDTPS = PS1048576 // use largest wdt value
#pragma config WINDIS = OFF // use non-window mode wdt
#pragma config FWDTEN = OFF // wdt disabled
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%

// DEVCFG2 - get the sysclk clock to 48MHz from the 8MHz fast rc internal oscillator
#pragma config FPLLIDIV = DIV_2 // divide input clock to be in range 4-5MHz
#pragma config FPLLMUL = MUL_24 // multiply clock after FPLLIDIV
#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL to get 48MHz

// DEVCFG3
#pragma config USERID = 0 // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = OFF // allow multiple reconfigurations
#pragma config IOL1WAY = OFF // allow multiple reconfigurations

#define PIC32_SYS_FREQ 48000000
#define PIC32_DESIRED_BAUD 230400

#define PIC32_CP0_FREQ 24000000
#define SIN_FREQ 2
#define TRI_FREQ 1

// initialize SPI1
void initSPI() {
    // Pin B14 has to be SCK1
    RPB14Rbits.RPB14R = 0b01111; //B14 as C1OUT
    // Turn of analog pins
    //...
    // Make an output pin for CS
    TRISAbits.TRISA2 = 0; //A2 is output
    LATAbits.LATA2 = 1; //turn A2 on
    // Set SDO1
    RPB11Rbits.RPB11R = 0b0011; //RB11 as SDO1
    // Set SDI1
    SDI1Rbits.SDI1R = 0b0100; //RB8 as SDI1

    // setup SPI1
    SPI1CON = 0; // turn off the spi module and reset it
    SPI1BUF; // clear the rx buffer by reading from it
    SPI1BRG = 1000; // 1000 for 24kHz, 1 for 12MHz; // baud rate to 10 MHz [SPI1BRG = (48000000/(2*desired))-1]
    SPI1STATbits.SPIROV = 0; // clear the overflow bit
    SPI1CONbits.CKE = 1; // data changes when clock goes from hi to lo (since CKP is 0)
    SPI1CONbits.MSTEN = 1; // master operation
    SPI1CONbits.ON = 1; // turn on spi 
}


// send a byte via spi and return the response
unsigned char spi_io(unsigned char o) {
  SPI1BUF = o;
  while(!SPI1STATbits.SPIRBF) { // wait to receive the byte

  }
  return SPI1BUF;
}

//Sets the DAC output voltage
//vout is 0 to 255 for 0 to 3.3V
//output is 0 for A, 1 for B
void write_dac(unsigned char vout, unsigned char output) {
    unsigned char byte1 = 0b0110000 + (output << 7) + (vout >> 4);
    unsigned char byte2 = 0 + (output << 4);
    LATAbits.LATA2 = 0;
    spi_io(byte1);
    spi_io(byte2);
    LATAbits.LATA2 = 1;
}

void initUart() {
    //setup UART pins
    RPB15Rbits.RPB15R = 0b0001; //B15 -> U1TX
    U1RXRbits.U1RXR = 0b0011; //B13 -> U1RX
    
    //setup/enable UART
    // turn on UART3 without an interrupt
    U1MODEbits.BRGH = 0; // set baud to NU32_DESIRED_BAUD
    U1BRG = ((PIC32_SYS_FREQ / PIC32_DESIRED_BAUD) / 16) - 1;

    // 8 bit, no parity bit, and 1 stop bit (8N1 setup)
    U1MODEbits.PDSEL = 0;
    U1MODEbits.STSEL = 0;

    // configure TX & RX pins as output & input pins
    U1STAbits.UTXEN = 1;
    U1STAbits.URXEN = 1;

    // enable the uart
    U1MODEbits.ON = 1;
}

// Write a character array using UART1
void PIC32_WriteUART1(const char * string) {
  while (*string != '\0') {
    while (U1STAbits.UTXBF) {
      ; // wait until tx buffer isn't full
    }
    U1TXREG = *string;
    ++string;
  }
}

int main() {

    __builtin_disable_interrupts(); // disable interrupts while initializing things

    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;
    
    initUart();
    initSPI();
    _CP0_SET_COUNT(0);

    __builtin_enable_interrupts();

    while (1) {
        int time = _CP0_GET_COUNT();
        if (time > LONG_MAX/2) {
            _CP0_SET_COUNT(_CP0_GET_COUNT() - INT_MAX/2);
            PIC32_WriteUART1("Long max / 2 reached!");
        }
        unsigned char voutA = 128 + (128.0*sin(time%(SIN_FREQ * PIC32_CP0_FREQ) / ((SIN_FREQ * PIC32_CP0_FREQ)/(2*3.14159265))));//128; //0 to 255 for 0 to 3.3V
        //PIC32_WriteUART1("Sin uploaded");
        long triPos = time%(TRI_FREQ * PIC32_CP0_FREQ);
        unsigned char voutB = triPos < (TRI_FREQ * PIC32_CP0_FREQ)/2 ?
            255.0 * (triPos / ((TRI_FREQ * PIC32_CP0_FREQ)/2.0)) :
            255.0-(255.0 * ((triPos - (TRI_FREQ * PIC32_CP0_FREQ/2.0)) / (TRI_FREQ * PIC32_CP0_FREQ/2.0)));//255;
        write_dac(voutA, 0);
        write_dac(voutB, 1);

    }
}