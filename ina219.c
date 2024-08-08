#include "ina219.h"

#define INA219_ADDR 0b1000000 // I2C address
#define INA219_REG_CONFIG 0x00 // config register address
#define INA219_REG_CURRENT 0x04 // current register
#define INA219_REG_CALIBRATION 0x05 // calibration register

//  Initialize I2C1 and the INA219 current sensor
void INA219_Startup() {
  // disable interrupts
  __builtin_disable_interrupts();

  // turn on I2C1 at 400kHz. (SET PINS)SDA1 is D9, SCL1 is D10
  i2c_master_setup();

  // set the INA219 sensitivity - 10 bit, plus/minus160mV, 148us per sample
  unsigned short ina219_calValue = 1024;
  unsigned short ina219_config = 0b0011000010001111;
  writeINA219(INA219_REG_CALIBRATION, ina219_calValue);
  writeINA219(INA219_REG_CONFIG, ina219_config);

  __builtin_enable_interrupts();
}

// get the current in mA
float INA219_read_current(){
  signed short value = readINA219(INA219_REG_CURRENT);
  float ma = value / 3.0;
  return ma;
}

// write 2 bytes
void writeINA219(unsigned char reg, unsigned short value){
  i2c_master_start();
  i2c_master_send(INA219_ADDR<<1); // write to the INA219
  i2c_master_send(reg); // the reg to write to
  i2c_master_send(value>>8);
  i2c_master_send(value&0xff);
  i2c_master_stop();
}

// read 2 bytes
signed short readINA219(unsigned char reg){
  i2c_master_start();
  i2c_master_send(INA219_ADDR<<1); // write to the INA219
  i2c_master_send(reg); // the reg to read from
  i2c_master_restart();
  i2c_master_send((INA219_ADDR<<1)|0b1); // read from the INA219
  unsigned char r1 = i2c_master_recv();
  i2c_master_ack(0); // read again
  unsigned char r2 = i2c_master_recv();
  i2c_master_ack(1); // no more reads
  i2c_master_stop();

  signed short value = (r1<<8)|r2;
  return value;
}

