

#ifndef I2C_ROTARY_H
#define I2C_ROTARY_H

#include "Arduino.h"
#include "Wire.h"

class I2c_Rotary {
public:
  I2c_Rotary(TwoWire* _i2c, uint8_t _address);
  int getEncoderPos(uint16_t* encPos, uint8_t* btnCnt);
  void setRGB(uint8_t red, uint8_t green, uint8_t blue);
private:
  TwoWire* i2c;
  uint8_t address;
};


#endif //I2C_ROTARY_H