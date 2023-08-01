#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include "Adafruit_MAX31855.h"
#include "Adafruit_MCP9808.h"
#include "Vrekrer_scpi_parser.h"
#include "InputDebounce.h"
#include "Logging.h"

#ifdef DISPLAY_SUPPORT
#include <LiquidCrystal_PCF8574.h>
#include <menu.h>               //menu macros and objects
#include <menuIO/PCF8574Out.h>  //arduino I2C LCD
#include <menuIO/keyIn.h>       //keyboard driver and fake stream (for the encoder button)
#include <menuIO/chainStream.h> // concatenate multiple input streams (this allows adding a button to the encoder)
#include <menuIO/serialIn.h>
#include "encoder_i2c.h"

using namespace Menu;
#endif

#ifdef DISPLAY_SUPPORT
LiquidCrystal_PCF8574 lcd(0x27, 0, 1, 2, 4, 5, 6, 7, 3);
// Encoder /////////////////////////////////////
#define ENCODER_ADDR  0x34
I2c_Rotary enc(&Wire, ENCODER_ADDR);
#define ENC_SENSIVITY 4
encoderI2cInStream encStream(&enc);
bool lcd_available = false;
// LCD geometry
const int LCD_COLS = 16;
const int LCD_ROWS = 2;

result idle(menuOut& o, idleEvent e);
result saveConfig(eventMask e, navNode& nav, prompt& item);
void LcdPrintTemperatures(void);

// input from the encoder + encoder button
MENU_INPUTS(in, &encStream);

#define MAX_DEPTH 2
MENU_OUTPUTS(out, MAX_DEPTH, LCD_OUT(lcd, { 0, 0, 16, 2 }), NONE);

CHOOSE(csv_delimiter, csvDelimiterMenu, "Delimiter", doNothing, noEvent, wrapStyle,
       VALUE(";", ';', doNothing, noEvent),
       VALUE(",", ',', doNothing, noEvent));

CHOOSE(decimal_separator, separatorMenu, "dec separator", doNothing, noEvent, wrapStyle,
       VALUE(".", '.', doNothing, noEvent),
       VALUE(",", ',', doNothing, noEvent));

CHOOSE(sd_card_detected, sdMenu, "SDcard", doNothing, noEvent, noStyle,
       VALUE("TURE", true, doNothing, noEvent),
       VALUE("FALSE", false, doNothing, noEvent));

MENU(mainMenu, "Main menu", doNothing, noEvent, wrapStyle,
     FIELD(logging_interval, "interval", "s", 1, 300, 10, 1, doNothing, enterEvent, noStyle),
     SUBMENU(csvDelimiterMenu),
     SUBMENU(separatorMenu),
     SUBMENU(sdMenu),
     OP("Save Config", saveConfig, enterEvent),
     EXIT("<Back"));

NAVROOT(nav, mainMenu, MAX_DEPTH, in, out); // the navigation root object

result idle(menuOut& o, idleEvent e) {
  switch (e) {
  case idleStart:
    o.print("suspending menu!");
    break;
  case idling:
    o.clear();
    break;
  case idleEnd:
    o.print("resuming menu.");
    break;
  }
  return proceed;
}

result saveConfig(eventMask e, navNode& nav, prompt& item) {
  lcd.clear();
  lcd.setCursor(0, 0);
  if (SdLogWriteConfig()) {
    lcd.print("File safed!");
  } else {
    lcd.print("Safe failed!");
  }
  delay(1000);
  mainMenu.dirty = true;
  return proceed;
}
#endif

void Read_temps();
void Identify(SCPI_C commands, SCPI_P parameters, Stream& interface);
void GetTemperature(SCPI_C commands, SCPI_P parameters, Stream& interface);
void GetTemperatureIntern(SCPI_C commands, SCPI_P parameters, Stream& interface);
void StartButtonPressedCbk(uint8_t pinIn);

static InputDebounce buttonStart;

SPIClassSAMD SPI2(&sercom1, PIN_SPI2_MISO, PIN_SPI2_SCK, PIN_SPI2_MOSI, PAD_SPI2_TX, PAD_SPI2_RX);

// Initialize the Thermocouple
Adafruit_MAX31855 tc_1(nCS_1, &SPI2);
Adafruit_MAX31855 tc_2(nCS_2, &SPI2);
Adafruit_MAX31855 tc_3(nCS_3, &SPI2);
Adafruit_MAX31855 tc_4(nCS_4, &SPI2);

Adafruit_MCP9808 ts[8];
bool ts_active[8];

SCPI_Parser scpi_instrument;

unsigned long TaskTimer;
unsigned long Task100msCnt;
unsigned long Task500msCnt;
unsigned long Task1000msCnt;

double temperature_ch_1;
double temperature_ch_2;
double temperature_ch_3;
double temperature_ch_4;

double temperature_int_ch_1;
double temperature_int_ch_2;
double temperature_int_ch_3;
double temperature_int_ch_4;

float temperature_ts[8];

void setup() {
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);
  SdLogBegin();

  pinMode(SD_CARD_DET, INPUT_PULLUP);
  pinMode(START_BUTTON_PIN, INPUT_PULLUP);
  buttonStart.registerCallbacks(StartButtonPressedCbk, nullptr, nullptr, nullptr);
  buttonStart.setup(START_BUTTON_PIN, BUTTON_DEBOUNCE_DELAY, InputDebounce::PIM_INT_PULL_UP_RES);

  SerialUSB.begin(9600);
  // while (!SerialUSB) {
  //   ; // wait for SerialUSB port to connect. Needed for native USB port only
  // }
  scpi_instrument.RegisterCommand("*IDN?", &Identify);
  scpi_instrument.RegisterCommand(":TEMPerature?", &GetTemperature);
  scpi_instrument.RegisterCommand(":TEMPerature:INTern?", &GetTemperatureIntern);

  // wait for MAX chip to stabilize
  // SerialUSB.print("Initializing sensor...");
  tc_1.begin();
  tc_2.begin();
  tc_3.begin();
  tc_4.begin();

  // scan for MCP9808 I2C sensors
  for (int i = 0; i < 8; i++) {
    if (ts[i].begin(0x18 + i)) {
      ts_active[i] = true;
      ts[i].setResolution(MCP_RESOLUTION);
      ts[i].wake();
    } else {
      ts_active[i] = false;
    }
  }

  // Init Task timer variables
  TaskTimer = millis();
  Task100msCnt = TaskTimer;
  Task500msCnt = TaskTimer;
  Task1000msCnt = TaskTimer;

  Read_temps();
  SPI.begin();

#ifdef DISPLAY_SUPPORT
  // detect display unit
  Wire.beginTransmission(ENCODER_ADDR);
  byte error = Wire.endTransmission();
  if (error != 0) {
    // display unit not available
    lcd_available = false;
  } else {
    lcd_available = true;

    // Display init
    lcd.begin(LCD_COLS, LCD_ROWS);
    lcd.setBacklight(255);
    lcd.setCursor(0, 0);

    encStream.init();
    nav.idleTask = idle;

    mainMenu[3].disable();
    nav.idleOn();
  }
#endif
}

void loop() {
  // get time
  TaskTimer = millis();

  // 100ms Task
  if (TaskTimer - Task100msCnt >= 100) {
    Task100msCnt += 100;
    SdLog100msTask();
  }

  // 500ms Task
  if (TaskTimer - Task500msCnt >= 500) {
    Task500msCnt += 500;
    Read_temps();
#ifdef DISPLAY_SUPPORT
    if (nav.sleepTask && lcd_available) {
      LcdPrintTemperatures();
    }
#endif
  }

  // 1000ms Task
  if (TaskTimer - Task1000msCnt >= 1000) {
    Task1000msCnt += 1000;
    SdLog1000msTask();
  }

  // idle task
  scpi_instrument.ProcessInput(SerialUSB, "\n");
  buttonStart.process(TaskTimer);
#ifdef DISPLAY_SUPPORT
  if (lcd_available)
    nav.poll();
#endif
}

void Read_temps() {
  temperature_int_ch_1 = tc_1.readInternal();
  temperature_ch_1 = tc_1.readCelsius();

  temperature_int_ch_2 = tc_2.readInternal();
  temperature_ch_2 = tc_2.readCelsius();

  temperature_int_ch_3 = tc_3.readInternal();
  temperature_ch_3 = tc_3.readCelsius();

  temperature_int_ch_4 = tc_4.readInternal();
  temperature_ch_4 = tc_4.readCelsius();

  for (int i = 0; i < 8; i++) {
    if (ts_active[i])
      temperature_ts[i] = ts[i].readTempC();
  }
}

void Identify(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  //*IDN?
  // Returns the instrument's identification string
  interface.println(F("MAX31855USB,v1.0"));
}

void GetTemperature(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  // :TEMPerature?
  // Returns the temperature of all 4 channels
  interface.print(temperature_ch_1);
  interface.print(",");
  interface.print(temperature_ch_2);
  interface.print(",");
  interface.print(temperature_ch_3);
  interface.print(",");
  interface.print(temperature_ch_4);
  for (int i = 0; i < 8; i++) {
    if (ts_active[i]) {
      interface.print(",");
      interface.print(temperature_ts[i]);
    }
  }
  interface.println("");
}

void GetTemperatureIntern(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  // :TEMPerature:INTern?
  // Returns the internal temperature of all 4 channels
  interface.print(temperature_int_ch_1);
  interface.print(",");
  interface.print(temperature_int_ch_2);
  interface.print(",");
  interface.print(temperature_int_ch_3);
  interface.print(",");
  interface.println(temperature_int_ch_4);
}

#ifdef DISPLAY_SUPPORT
void LcdPrintTemperatures(void) {
  lcd.setCursor(0, 0);
  lcd.print(temperature_ch_1);
  lcd.print("C  ");
  lcd.setCursor(8, 0);
  lcd.print(temperature_ch_2);
  lcd.print("C  ");
  lcd.setCursor(0, 1);
  lcd.print(temperature_ch_3);
  lcd.print("C  ");
  lcd.setCursor(8, 1);
  lcd.print(temperature_ch_4);
  lcd.print("C  ");
}
#endif