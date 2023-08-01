//
// Created by Friedrich on 19.07.2021.
//

#ifndef MAX31855USB_LOGGING_H
#define MAX31855USB_LOGGING_H
#include "Arduino.h"
#include "SD.h"
#include "SDConfig.h"
#include "config.h"

bool SdLogBegin();
bool SdLogInit();
void SdLog1000msTask();
void SdLog100msTask();
bool SdLogWriteConfig();

extern int logging_interval;
extern char csv_delimiter;
extern char decimal_separator;
extern bool sd_card_detected;

#endif //MAX31855USB_LOGGING_H
