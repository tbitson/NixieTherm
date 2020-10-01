/******************************************************
   config.h

   Global settings saved and loaded from
     non-volatile memory (EEPROM).
   Set the default values if EEPROM contents do
   not match the current version.

   version 1.0.8   09May2020

   Notes:
   SN1 Cal:
   SN2 Cal: 718/3400
   SN3 Cal: 786/2940



 ********************************************************/

#ifndef CONFIG_H
#define CONFIG_H

#include <Strings.h>


// prototypes
bool saveConfig();
bool loadConfig();
void loadDefaults();
void printConfig();
void clearEEPROM();
void showEEPROM();



// if the first byte of stored data matches this, it
// is assumed valid data for this version
#define EEPROM_VERSION 109
#define EEPROM_ADDR    0


// the settings saved to non-volatile memory
struct Config
{
  byte     vers;
  String   serialNum;
  uint16_t displayColor;
  float    calValLow;
  float    calValHigh;
  bool     calComplete;
};



// create instance of configuration struct
Config cfg;


bool loadConfig()
{
  byte version;

  // init eeprom library
  EEPROM.begin();
  delay(100);

  // check version of stored config
  EEPROM.get(EEPROM_ADDR, version);
  Serial.print("EEPROM Stored Version   = "); Serial.println(version);
  Serial.print("EEPROM Required Version = "); Serial.println(EEPROM_VERSION);

  if (version == EEPROM_VERSION)
  {
    EEPROM.get(EEPROM_ADDR, cfg);
    delay(200);
    Serial.println("Stored Config loaded");
    Serial.print("loaded "); Serial.print(sizeof(cfg)); Serial.println(" bytes");
    return true;
  }
  else
  {
    Serial.println("Config NOT loaded, using defaults");
    loadDefaults();
    saveConfig();
  }

  return false;
}



// defaults for first power-up
void loadDefaults()
{
  //clearEEPROM();

  // set up defaults
  cfg.vers         = EEPROM_VERSION;
  cfg.displayColor = BLU;
  cfg.calComplete  = false;



#ifdef SN1
  cfg.serialNum = "SN1";
  cfg.calValLow    = 702;
  cfg.calValHigh   = 3244;
#endif

#ifdef SN2
  cfg.serialNum = "SN2";
  cfg.calValLow    = 750;
  cfg.calValHigh   = 2900;
#endif

#ifdef SN3
  cfg.serialNum = "SN3";
  cfg.calValLow    = 750;
  cfg.calValHigh   = 2900;
#endif

  saveConfig();
}



bool saveConfig()
{
  // don't store if version is 0, used for dev to force defaults
  if (EEPROM_VERSION == 0)
    return false;

  cfg.vers = EEPROM_VERSION;
  EEPROM.put(EEPROM_ADDR, cfg);
  Serial.print("Saved "); Serial.print(sizeof(cfg)); Serial.println(" bytes");
  return true;
}


void printConfig()
{
  Serial.print("cfg.vers        : "); Serial.println(cfg.vers);
  Serial.print("cfg.displayColor: "); Serial.println(cfg.displayColor);
  Serial.print("cfg.calComplete : "); Serial.println(cfg.calComplete);
  Serial.print("cfg.serialNum   : "); Serial.println(cfg.serialNum);
  Serial.print("cfg.calValLow   : "); Serial.println(cfg.calValLow);
  Serial.print("cfg.calValHigh  : "); Serial.println(cfg.calValHigh);
  Serial.println();
}


// clear eeprom contents (dev use only so far)
void clearEEPROM()
{
  Serial.println("Clearing EEPROM...");

  for (int i = 0 ; i < EEPROM.length() ; i++)
    EEPROM.write(i, 0xFF);

  Serial.println("done");
}



// show eeprom contents (dev use only so far)
void showEEPROM()
{
  byte b;

  Serial.println("EEPROM contents:");

  for (int i = 0 ; i < EEPROM.length() ; i++)
  {
    EEPROM.get(i, b);

    Serial.print("Addr 0x");
    Serial.print(i, HEX);
    Serial.print(" = ");
    Serial.println(b);
  }

  Serial.println();
}

#endif
