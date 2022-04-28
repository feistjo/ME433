#include"i2c_master_noint.h"

#define MCP_ADDRESS 0b0100000
#define MCP_IODIR 0x00
#define MCP_IPOL 0x01
#define MCP_GPINTEN 0x02
#define MCP_DEFVAL 0x03
#define MCP_INTCON 0x04
#define MCP_GPPU 0x06
#define MCP_GPIO 0x09
#define MCP_OLAT 0x0A

void mcp23008_write(unsigned char ad, unsigned char reg, unsigned char val);
unsigned char mcp23008_read(unsigned char address, unsigned char reg);
void setMCPPin(unsigned char address, unsigned char reg, unsigned char value);