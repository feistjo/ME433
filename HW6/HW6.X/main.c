#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include"MCP23008.h"

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

    // do your TRIS and LAT commands here, not finished
    TRISAbits.TRISA4 = 0; //A4 is output
    LATAbits.LATA4 = 0; //turn A4 off
    TRISBbits.TRISB4 = 1; //B4 is input
    
    //i2c setup
    i2c_master_setup();
    //init the mcp23008 with gp7 as output and gp0 as input
    mcp23008_write(MCP_ADDRESS, MCP_IODIR, 0b01111111);
    //turn on gp7
    mcp23008_write(MCP_ADDRESS, MCP_OLAT, 0b10000000);
    //set gp0 as pullup
    mcp23008_write(MCP_ADDRESS, MCP_GPPU, 0b00000001);

    __builtin_enable_interrupts();
    
    _CP0_SET_COUNT(0);
    while (1) {
        // use _CP0_SET_COUNT(0) and _CP0_GET_COUNT() to test the PIC timing
        // remember the core timer runs at half the sysclk
        
        if(_CP0_GET_COUNT() >= 24000000) {
            LATAbits.LATA4 = !LATAbits.LATA4; //A4 switch on/off
            //mcp23008_write(MCP_ADDRESS, MCP_OLAT, 0b10000000); //turn on GP7
            _CP0_SET_COUNT(0);
        }
        /*
        if(_CP0_GET_COUNT() >= 12000000) {
            mcp23008_write(MCP_ADDRESS, MCP_OLAT, 0b00000000); //turn off GP7
        }*/

        if(!(mcp23008_read(MCP_ADDRESS, MCP_GPIO) && 0b00000001)) { //if GP0 low
            mcp23008_write(MCP_ADDRESS, MCP_OLAT, 0b10000000); //turn on GP7
        } else {
            mcp23008_write(MCP_ADDRESS, MCP_OLAT, 0b00000000); //turn off GP7
        }
    }
}