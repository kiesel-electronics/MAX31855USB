//
// Created by Friedrich on 19.07.2021.
//

#ifndef MAX31855USB_CONFIG_H
#define MAX31855USB_CONFIG_H

// digital IO pins.
#define nCS_1   5   //PA15
#define nCS_2   2   //PA14
#define nCS_3   18  //PA05
#define nCS_4   17  //PA04

#define LED     6 //PA20

#define PIN_SPI2_MISO 34  //PA19
#define PIN_SPI2_SCK  37  //PA17
#define PIN_SPI2_MOSI 35  //PA16
#define PAD_SPI2_TX   SPI_PAD_0_SCK_1
#define PAD_SPI2_RX   SERCOM_RX_PAD_3

#define MCP_RESOLUTION 3
// Mode Resolution SampleTime
//  0    0.5°C       30 ms
//  1    0.25°C      65 ms
//  2    0.125°C     130 ms
//  3    0.0625°C    250 ms

#define SD_CARD_nCS   38  //PA13
#define SD_CARD_DET   16  //PB09
#define START_BUTTON_PIN    39  //PA21

#define BUTTON_DEBOUNCE_DELAY   20   // [ms]

#define DBG_EN

#ifdef DBG_EN
#define DBG_PRINT(x)   SerialUSB.println(x);
#else
#define DBG_PRINT(x)
#endif

#endif //MAX31855USB_CONFIG_H
