#include"MCP23008.h"

//start bit
    //write the address with a write bit
    //write the reg to change
    //write the value to put in that reg
    //stop bit
    
void mcp23008_write(unsigned char ad, unsigned char reg, unsigned char val) {
    i2c_master_start();
    i2c_master_send(ad<<1); //for write
    i2c_master_send(reg);
    i2c_master_send(val);
    i2c_master_stop();
}

unsigned char mcp23008_read(unsigned char address, unsigned char reg) {
    i2c_master_start();
    i2c_master_send(address<<1); //for write
    i2c_master_send(reg);
    i2c_master_restart();
    i2c_master_send((address<<1)|0b1); //for read
    unsigned char r = i2c_master_recv();
    i2c_master_ack(1); //done
    i2c_master_stop();
    
    return r;
}