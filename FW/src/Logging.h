//
// Created by Friedrich on 19.07.2021.
//

#ifndef MAX31855USB_LOGGING_H
#define MAX31855USB_LOGGING_H
#include "Arduino.h"
#include "SD.h"
#include "SDConfig.h"
#include "config.h"

void SdLogBegin();
bool SdLogInit();
void SdLog1000msTask();
void SdLog100msTask();

#endif //MAX31855USB_LOGGING_H
