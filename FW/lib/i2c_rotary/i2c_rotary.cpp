
#include "i2c_rotary.h"


I2c_Rotary::I2c_Rotary(TwoWire* _i2c, uint8_t _address) :i2c(_i2c), address(_address) {}

int I2c_Rotary::getEncoderPos(uint16_t* encPos, uint8_t* btnCnt) {
  uint16_t enc_reading = 0;
  uint8_t btn_reading = 0;

  uint8_t ret = i2c->requestFrom(address, 3);
  if (3 <= i2c->available()) { // if two bytes were received
    enc_reading = i2c->read();  // receive low byte (overwrites previous reading)
    enc_reading |= i2c->read() << 8; // receive high byte as lower 8 bits
    btn_reading = i2c->read();
    *encPos = enc_reading;
    *btnCnt = btn_reading;
  } else {
    ret = -1;
  }
  return ret;
}

void I2c_Rotary::setRGB(uint8_t red, uint8_t green, uint8_t blue) {
  i2c->beginTransmission(address);
  i2c->write(red);
  i2c->write(green);
  i2c->write(blue);
  i2c->endTransmission();
}



