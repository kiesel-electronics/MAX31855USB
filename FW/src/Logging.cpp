//
// Created by Friedrich on 19.07.2021.
//
#include "Logging.h"


char fileName[20];
long fileNum = 1;
char configFile[] = "config.txt"; // configuration file for SD card recording
char csv_delimiter = ';';
char decimal_separator = '.';
bool sd_card_detected;
unsigned int dataLogTimer;
int logging_interval_cnt = 0;
int logging_interval = 1;
bool logging_active;
int ledTimer = 0;

extern double temperature_ch_1;
extern double temperature_ch_2;
extern double temperature_ch_3;
extern double temperature_ch_4;


void incFileNum() { // generate next file name:
  String s = "dat" + String(++fileNum) + ".csv";
  s.toCharArray(fileName, 20);
}


bool SdLogBegin() {
  logging_active = false;

  if (!SD.begin(SD_CARD_nCS)) {
    sd_card_detected = false;
    return false;
  }
  sd_card_detected = true;
  // try reading the config file
  int maxLineLength = 127;
  SDConfig cfg;
  if (!cfg.begin(configFile, maxLineLength)) {
    return false;
  }
  // set default values
  csv_delimiter = ';';
  decimal_separator = '.';
  logging_interval = 1;
  // Read each setting from the file.
  while (cfg.readNextSetting()) {
    // Put a nameIs() block here for each setting you have.
    // doDelay
    if (cfg.nameIs("csv_delimiter")) {
      csv_delimiter = cfg.getCharValue();
    } else if (cfg.nameIs("decimal_separator")) {
      decimal_separator = cfg.getCharValue();
    } else if (cfg.nameIs("logging_interval")) {
      logging_interval = cfg.getIntValue();
    }
  }
  cfg.end();
  DBG_PRINT("delimiter: " + String(csv_delimiter));
  DBG_PRINT("separator: " + String(decimal_separator));
  DBG_PRINT("interval: " + String(logging_interval));
  return true;
}

bool SdLogInit() {

  // search for next filename
  fileNum = 0;
  incFileNum();
  while (SD.exists(fileName)) incFileNum();
  DBG_PRINT("new file name: " + String(fileName));
  dataLogTimer = 0;
  // open new file and generate first line
  File dataFile = SD.open(fileName, FILE_WRITE);
  if (!dataFile) {
    return false;
  }
  String dataString = "";
  dataString += "Time/s" + String(csv_delimiter);
  dataString += "tc1/C" + String(csv_delimiter);
  dataString += "tc2/C" + String(csv_delimiter);
  dataString += "tc3/C" + String(csv_delimiter);
  dataString += "tc4/C" + String(csv_delimiter);
  dataFile.println(dataString);
  dataFile.close();
  return true;
}


bool SdLogTemp() {
  String dataString = "";
  String valueString = "";
  if (sd_card_detected) {
    dataString += String(dataLogTimer);
    dataString += String(csv_delimiter);

    valueString = String(temperature_ch_1);
    valueString.replace('.', decimal_separator);
    dataString += valueString;
    dataString += String(csv_delimiter);

    valueString = String(temperature_ch_2);
    valueString.replace('.', decimal_separator);
    dataString += valueString;
    dataString += String(csv_delimiter);

    valueString = String(temperature_ch_3);
    valueString.replace('.', decimal_separator);
    dataString += valueString;
    dataString += String(csv_delimiter);

    valueString = String(temperature_ch_4);
    valueString.replace('.', decimal_separator);
    dataString += valueString;

    File dataFile = SD.open(fileName, FILE_WRITE);
    if (dataFile) {
      dataFile.println(dataString);
      dataFile.close();
      return true;
    }
  }
  return false;
}


void StartButtonPressedCbk(uint8_t pinIn) {
  logging_active = !logging_active;
  if (logging_active) {
    DBG_PRINT("Start Logging!");
    // detect SD Card
    if (digitalRead(SD_CARD_DET) == LOW) {
      // SD card detected!
      DBG_PRINT("SD card detected!");
      sd_card_detected = true;
      if (!SdLogInit()) {
        DBG_PRINT("SD init failed!");
        logging_active = false;
      }
      if (SdLogTemp()) {
        // sd write successful
        // switch LED on (will be switched off in the next 100ms cycle to create a short blink)
        digitalWrite(LED, LOW);
        ledTimer = 1;
      }
      logging_interval_cnt = 0;
    } else {
      DBG_PRINT("No SD card detected!");
      logging_active = false;
      sd_card_detected = false;
    }
  } else {
    DBG_PRINT("Stop Logging!");
  }
}


void SdLog1000msTask() {
  if (logging_active) {
    logging_interval_cnt++;
    dataLogTimer++;
    if (logging_interval_cnt >= logging_interval) {
      if (SdLogTemp()) {
        // sd write successful
        // switch LED on (will be switched off in the next 100ms cycle to create a short blink)
        digitalWrite(LED, LOW);
        ledTimer = 1;
      }
      logging_interval_cnt = 0;
    }
  } else {
    logging_interval_cnt = 0;
    dataLogTimer = 0;
  }
}


void SdLog100msTask() {
  if (ledTimer == 0) {
    // switch LED off
    digitalWrite(LED, HIGH);
  } else {
    ledTimer--;
  }

}


bool SdLogWriteConfig() {
  SD.remove(configFile);
  File cfgFile = SD.open(configFile, FILE_WRITE);
  if (cfgFile) {
    cfgFile.print("csv_delimiter=");
    cfgFile.println(csv_delimiter);

    cfgFile.print("decimal_separator=");
    cfgFile.println(decimal_separator);

    cfgFile.print("logging_interval=");
    cfgFile.println(logging_interval);

    cfgFile.close();
    return true;
  } else {
    return false;
  }
}