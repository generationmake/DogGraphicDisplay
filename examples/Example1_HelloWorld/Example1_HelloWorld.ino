/******************************************************************************
  Displays "Hello World" on the display

  bernhard@generationmake.de
  Original Creation Date: Oct. 18, 2019

  This code is Beerware; if you see me at the local,
  and you've found our code helpful, please buy us a round!

  Hardware Connections:
  Connect DOGM128-6 to Arduino UNO. Use Hardware SPI.
  SI    = 11 (Hardware SPI)
  SCLK  = 13 (Hardware SPI)
  CS    = 6
  A0    = 8
  RESET = 9
  Backlight (if needed) is connected via a transistor to pin 10)

  Distributed as-is; no warranty is given.
******************************************************************************/
#include <DogGraphicDisplay.h>
#include "ubuntumono_b_16.h"

#define BACKLIGHTPIN 10

DogGraphicDisplay DOG;

void setup() {
  pinMode(BACKLIGHTPIN,  OUTPUT);   // set backlight pin to output
  digitalWrite(BACKLIGHTPIN,  HIGH);  // enable backlight pin

  DOG.begin(6,0,0,8,9,DOGM128);   //CS = 6, 0,0= use Hardware SPI, A0 = 8, RESET = 9, EA DOGM128-6 (=128x64 dots)

  DOG.clear();  //clear whole display

  DOG.string(0,3,UBUNTUMONO_B_16,"Hello World",ALIGN_CENTER); // print "Hello World" in line 3, centered
}

void loop() {
  // nothing in here
}
