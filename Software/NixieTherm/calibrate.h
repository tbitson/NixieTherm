/************************************************
   calibrate .h

   sets up DAC value for the nixie tube driver.


   Transistor for driving the dynamic cathode starts
   conduction at 0.7v. 0.7v =~ 869 counts on DAC, therefore:
   4096 - 869 = 3227 is usable calculated range of DAC.
   Testing shows that ~2400 counts is full scale with transistor
   gain. May opt to change cathode resistor
   compenstate for 0.7V drop in transistor
   full off = 4096 * (0.7 / 3.3) = 869
   full on = 4096, actually ends up 3269 based on
   transistor gain
   range ~= 3269 - 869 = 2400


 **********************************************/



#ifndef CALIBRATE_H
#define CALIBRATE_H





// calibration constants
const float fullOn    = 2900.0;
const float fullOff   = 700.0;
const float minTemp   = 60.0;
const float maxTemp   = 90.0;
const float calTempLo = 65.0;
const float calTempHi = 85.0;


extern void setDAC(float degs);


void calibrate()
{
  bool loop = true;

  Serial.println("Cal mode");
  Serial.println("Press switch 1 to decrease");
  Serial.println("Press switch 2 to increase");
  Serial.println("Press both to exit");

  while (loop)
  {
    while (digitalRead(SWITCH_2) || digitalRead(SWITCH_1))
    {
      // set dac for 65 degs
      setDAC(65);
      Serial.print("adjust for 65 Degs: ");

      if (digitalRead(SWITCH_1) == 0)
        cfg.calValLow -= 2;
      if (digitalRead(SWITCH_2) == 0)
        cfg.calValLow += 2;

      Serial.println(cfg.calValLow);
      delay(200);
    }

    delay(1000);

    while (digitalRead(SWITCH_2) || digitalRead(SWITCH_1))
    {
      // set dac to 85 degs
      setDAC(85);
      Serial.print("adjust for 85 Degs: ");

      if (digitalRead(SWITCH_1) == 0)
        cfg.calValHigh -= 2;
      if (digitalRead(SWITCH_2) == 0)
        cfg.calValHigh += 2;

      Serial.println(cfg.calValHigh);
      delay(200);
    }

    delay(500);

    if (digitalRead(SWITCH_2) == 0 && digitalRead(SWITCH_1) == 0)
      loop = false;
  }

  cfg.calComplete = true;

  saveConfig();

  Serial.print("Final Cal Values:");
  Serial.print("  min: "); Serial.print(cfg.calValLow);
  Serial.print("  max: "); Serial.print(cfg.calValHigh);
  Serial.println();
}

#endif
