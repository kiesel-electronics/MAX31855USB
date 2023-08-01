/* -*- C++ -*- */
/********************
*/

#ifndef RSITE_ARDUINO_MENU_ENCODER
#define RSITE_ARDUINO_MENU_ENCODER

#include <menuDefs.h>
#include "i2c_rotary.h"

namespace Menu {

  //emulate a stream based on encoderIn movement returning +/- for every 'sensivity' steps

  class encoderI2cInStream :public menuIn {
  private:
    I2c_Rotary* enc;
  public:
    int sensivity = 1;
    uint16_t oldPos = 0;
    uint8_t  oldBtnCnt = 0;

    void init(void) {
      uint16_t encPos;
      uint8_t btnCnt;
      enc->getEncoderPos(&encPos, &btnCnt);
      oldPos = encPos;
      oldBtnCnt = btnCnt;
    }

    encoderI2cInStream(I2c_Rotary* _enc) :enc{ _enc } {}

    inline void setSensivity(int s) {
      if (s >= 1)
        sensivity = s;
    }

    int available(void) {
      return peek() != -1;
    }

    uint16_t encPos;
    uint8_t btnCnt;
    int peek(void) override {
      int ret = enc->getEncoderPos(&encPos, &btnCnt);
      if (ret == -1)
        return -1;

      int8_t btnDiff = (int8_t)btnCnt - (int8_t)oldBtnCnt;       // button counter increases by 1 when pressed and 1 when released
      if ((btnDiff > 0) && (btnDiff % 2 == 0))  // make sure to trigger the event when the button is released
        return options->navCodes[enterCmd].ch;// menu::enterCode;

      int16_t encDiff = (int16_t)encPos - (int16_t)oldPos;
      if (encDiff <= -sensivity)
        return options->navCodes[downCmd].ch; // menu::downCode;
      if (encDiff >= sensivity)
        return options->navCodes[upCmd].ch;   // menu::upCode;
      return -1;
    }

    int read() override {
      int change = peek();
      if (change == options->navCodes[upCmd].ch)
        oldPos += sensivity;
      else if (change == options->navCodes[downCmd].ch)
        oldPos -= sensivity;
      else if (change == options->navCodes[enterCmd].ch)
        oldBtnCnt += 2;
      return change;
    }

    void flush() {
      uint16_t encPos;
      uint8_t btnCnt;
      enc->getEncoderPos(&encPos, &btnCnt);
      oldPos = encPos;
    }

    size_t write(uint8_t v) {
      oldPos = v;
      return 1;
    }
  };

}//namespace Menu
#endif
