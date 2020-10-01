/*************************************
    NixieTherm - Nixie IN-12 Thermometer
    Version 1.0.8  10Mayil2020  t.bitson

    Reads a DS18B20 1-wire temp sensor and
    displays results on a vintage Nixie IN-12
    bar-graph tube. Uses a Teensy 3.2 DAC ouput
    to drive the active cathode.

   Notes:
   12 bits = 4096 counts
   analogWriteResolution is set for 12 bit to accomodate
   Teensy's built-in DAC. This affects all analogWrite()
   functions, so the PWM for adjusting the LED brightness
   uses 12 bits.
   Use setLEDColor() for 1 of 6 basic colors or
   colorWheel12() for any of the 4096 colors


*****************************************************/

#define VERSION "NixieTherm Vers. 1.0.9  18Jun2020"



// uncomment to enable additional prints
//#define DEBUG

// select serial number for initial cal values
#define SN1
//#define SN2
//#define SN3


// hardware connections
// SN3 has unique wiring
#ifdef SN3
#define RED_LED      4
#define GRN_LED      3
#define BLU_LED      6
#else
#define RED_LED      4
#define GRN_LED      5
#define BLU_LED      6
#endif

#define PHOTO_CELL   A3
#define DAC_PIN      A14

#define SWITCH_1     7
#define SWITCH_2     8

#define ONE_WIRE_PIN 9
#define BUILTIN_LED  13

// states
#define ON      1
#define OFF     0


// global for interrupts
volatile uint16_t currentColor;   // current color


// program includes
#include <OneWire.h>
#include <Strings.h>
#include <EEPROM.h>

#include "leds.h"
#include "config.h"
#include "calibrate.h"
#include "interrupts.h"



// create instance of 1-wire temp sensor
OneWire ds(ONE_WIRE_PIN);

// global vars
bool calMode = false;     // flag to enter cal mode
uint32_t count = 0;       // loop counter
byte addr[8];             // 1-wire device address
int lastCounts = 0;        // last DAC setting in counts




//---------------------------------------------------------


void setup()
{
  // wait for things to settle
  delay(2000);

  // load config from nonvolatile memory
  loadConfig();


  Serial.println();
  Serial.println(VERSION);
  Serial.println(cfg.serialNum);
  Serial.println();

  // #ifdef DEBUG
  printConfig();
  // #endif

  // set up teensy DAC, drives nixie cathode
  analogWriteResolution(12);
  analogWrite(DAC_PIN, 0);

  // init hardware pins
  pinMode(RED_LED, OUTPUT);
  pinMode(GRN_LED, OUTPUT);
  pinMode(BLU_LED, OUTPUT);

  pinMode(BUILTIN_LED, OUTPUT);

  pinMode(SWITCH_1, INPUT_PULLUP);
  pinMode(SWITCH_2, INPUT_PULLUP);

  delay(500);

  // check for switch down at power up
  if (digitalRead(SWITCH_1) == 0)
    calMode = true;

  // check for switch down at power up
  if (digitalRead(SWITCH_2) == 0)
    testMode();

  // init DAC
  setDAC(60);

  // get temp sensor address
  getSensorAddress();

  // if first power up or cal mode selected
  if (calMode)
  {
    colorPicker(BLU);
    calibrate();
  }
  else
  {
    // cycle through all colors
    for (uint16_t i = 0; i < 4096; i++)
    {
      colorPicker(i);
      //delay(2);
    }
  }

  // cycle to selected color
  currentColor = cfg.displayColor;
  for (uint16_t i = 0; i < currentColor; i++)
  {
    colorPicker(i);
    delay(2);
  }

  // run through range
  exerciseNixie();

  // set up switch interrupts
  attachInterrupt(SWITCH_1, SW1_InterruptHandler, LOW);
  attachInterrupt(SWITCH_2, SW2_InterruptHandler, LOW);

  Serial.println("setup complete");
}




void loop()
{
  byte present = 0;
  byte data[12];
  float degsC, degsF;


  // exercise nixie approx once an hour
  if (count > 3600)
  {
    exerciseNixie();
    count = 0;
  }

  // led indicates we're getting the temperature
  digitalWrite(BUILTIN_LED, ON);

  ds.reset();           // reset 1-wire bus
  ds.select(addr);      // address device
  ds.write(0x44, 1);    // start conversion
  delay(1000);          // wait for temp conversion
  digitalWrite(BUILTIN_LED, OFF);

  present = ds.reset(); // reset bus & check if device is still present
  ds.select(addr);      // select device
  ds.write(0xBE);       // read temp data

  delay(100);

  if (present)
  {
    for (byte i = 0; i < 9; i++)
    {
      // we need 9 bytes
      data[i] = ds.read();
      //Serial.print(data[i], HEX);
      //Serial.print(" ");
    }

    uint8_t crc = OneWire::crc8(data, 8);
    //Serial.print(" calc CRC: ");
    //Serial.print(crc, HEX);
    //Serial.print(" should be: ");
    //Serial.println(data[8], HEX);

    if (data[8] != crc)
    {
      Serial.println("CRC Invalid");
      return;
    }

    int16_t raw = (data[1] << 8) | data[0];
    degsC = (float)raw / 16.0;
    degsF = degsC * 1.8 + 32.0;
    Serial.print("Temp = "); Serial.print(degsF, 1); Serial.println(" Degs F");
    setDAC(degsF);
    delay(5000);
  }
  else
  {
    Serial.println("Device not detected");
    delay(1000);
  }

  // increment loop counter
  count++;
}





void setDAC(float degs)
{
  int counts = 0;
  
  // full range is ~60 to ~90 degs, but calibrated at 65 and 85
  // calc 12-bit DAC value from temp

    float val = (degs - minTemp) * ((cfg.calValHigh - cfg.calValLow) / (maxTemp - minTemp)) + cfg.calValLow;
    counts = (int)val;
    Serial.print("counts = "); Serial.println(counts);

  // adjust nixie level incrementally, just for looks
  if (counts > lastCounts)
    for (int i = lastCounts; i < counts; i++)
    {
      analogWrite(DAC_PIN, i);
      delay(2);
    }

  if (counts < lastCounts)
    for (int i = lastCounts; i > counts; i--)
    {
      analogWrite(DAC_PIN, i);
      delay(2);
    }

  analogWrite(DAC_PIN, (int)counts);
  lastCounts = counts;
}




void getSensorAddress()
{
  // get the 1-wire temp sensor address
  // todo: store it in NV RAM
  //
  if (!ds.search(addr))
  {
    ds.reset_search();
    delay(250);
  }

  Serial.print("Temp Sensor Addr = {");
  for ( uint8_t i = 0; i < 8; i++)
  {
    Serial.print("0x");
    Serial.print(addr[i], HEX);
    Serial.print(", ");
  }
  Serial.println("}");

  if (OneWire::crc8(addr, 7) != addr[7])
    Serial.println("Warning: Invalid CRC");

  Serial.println();
  return;
}



void exerciseNixie()
{
  for (uint16_t i = fullOff; i < fullOn; i++)
  {
    analogWrite(DAC_PIN, i);
    delay(2);
  }

  for (uint16_t i = fullOn; i > fullOff; i--)
  {
    analogWrite(DAC_PIN, i);
    delay(2);
  }

  lastCounts = fullOff;
}




void testMode()
{
  Serial.println("test mode");
  
  while (true)
  {
    analogWrite(DAC_PIN, 700);
    delay(2000);
    analogWrite(DAC_PIN, 2900);
    delay(2000);

    if (digitalRead(SWITCH_1) == 0)
      return;
  }
}
