/******************************************************************************
  Displays a text on the display scrolling from right to left
  
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

dogGraphicDisplay DOG;

void setup() {
  pinMode(BACKLIGHTPIN,  OUTPUT);   // set backlight pin to output
  digitalWrite(BACKLIGHTPIN,  HIGH);  // enable backlight pin

  DOG.begin(6,0,0,8,9,DOGM128);   //CS = 6, 0,0= use Hardware SPI, A0 = 8, RESET = 9, EA DOGM128-6 (=128x64 dots)

  DOG.clear();  //clear whole display
}

void loop() {
  static int offset=DOG.display_width(); // static variable with the size of the display, so text starts at the right border

  DOG.string(offset,3,UBUNTUMONO_B_16,"Hello to the scrolling World!"); // print "Hello World" in line 3 at position offset

  offset--; // decrasye offset so text moves to the left
  if(offset<-232) offset=DOG.display_width(); //our text is 232 pixels wide so restart at this value

  delay(40);  // wait a little bit
}
