#ifndef INA219__H__
#define INA219__H__

#include <xc.h> // processor SFR definitions
#include <sys/attribs.h> // __ISR macro

#include "NU32DIP.h"
#include "i2c_master_noint.h"

void INA219_Startup();
float INA219_read_current();

void writeINA219(unsigned char, unsigned short);
signed short readINA219(unsigned char);

#endif // INA219__H__
