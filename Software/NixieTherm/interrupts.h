/**************************************
 * interrupts handlers
 * 
 * 
 * 
 */



#ifndef INTERRUPT_H
#define INTERRUPT_H

void SW1_InterruptHandler();
void SW2_InterruptHandler();




// switch interrupt handlers
void SW1_InterruptHandler()
{
  detachInterrupt(SWITCH_1);
  delay(20);
  Serial.println("Switch 1 Int");

  while (digitalRead(SWITCH_1) == 0)
  {
    // increment color
    currentColor++;

    // check bounds
    if (currentColor > 4095)
      currentColor = 0;

    // set color
    colorPicker(currentColor);
    delay(5);
  }

  cfg.displayColor = currentColor;
  saveConfig();
  attachInterrupt(SWITCH_1, SW1_InterruptHandler, LOW);
  return;
}



void SW2_InterruptHandler()
{
  detachInterrupt(SWITCH_2);
  delay(20);

  Serial.println("Switch 2 Int");

  // wait for button release
  while (digitalRead(SWITCH_2) == 0)
    delay(10);

  attachInterrupt(SWITCH_2, SW2_InterruptHandler, LOW);
  return;
}

#endif
