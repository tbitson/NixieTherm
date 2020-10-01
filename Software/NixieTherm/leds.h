/******************************************
   leds.h - functions for setting the
            led color

    version 1.0

 ******************************************/


#ifndef LED_H
#define LED_H



// LED basic colors
#define RED     0
#define YEL     682
#define GRN     1365
#define TEAL    2047
#define BLU     2730
#define VIO     3412

#define DEFAULT_COLOR  BLU


// 12-bit color picker
uint16_t colorPicker(uint16_t color)
{
#ifdef DEBUG
  Serial.print("color = "); Serial.println(color);
#endif

  // this version uses 12-bit color space
  // 0 = r -> 1365 = g -> 2730 = b -> 4095 r
  int wedge = 4095 / 3;   // 1365 counts per wedge

  if (color < wedge)                // R -> G
  {
    color *= 3;
    analogWrite(RED_LED, 4095 - color);
    analogWrite(GRN_LED, color);
    analogWrite(BLU_LED, 0);
  }
  else if (color < 2 * wedge)       // G -> B
  {
    color = (color - wedge) * 3;
    analogWrite(RED_LED, 0);
    analogWrite(GRN_LED, 4095 - color);
    analogWrite(BLU_LED, color);
  }
  else
  {
    color = (color - wedge * 2) * 3;  // B -> R
    analogWrite(RED_LED, color);
    analogWrite(GRN_LED, 0);
    analogWrite(BLU_LED, 4095 - color);
  }

return color;
}



void ledsOff()
{
  analogWrite(RED_LED, 0);
  analogWrite(GRN_LED, 0);
  analogWrite(BLU_LED, 0);
}

#endif
