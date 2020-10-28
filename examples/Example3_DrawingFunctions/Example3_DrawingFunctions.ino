/******************************************************************************
  Draws different shapes on the display
  
  Original Creation Date: Oct. 24, 2020

  This code is Beerware; if you see me at the local, 
  and you've found our code helpful, please buy us a round!

  Tested with ArduiHMI v1.3 and Arduino MKRZERO
  
  Distributed as-is; no warranty is given.
******************************************************************************/

#include <DogGraphicDisplay.h>

DogGraphicDisplay DOG;

void setup() {
  pinMode(A6, OUTPUT);   // set backlight pin to output
  digitalWrite(A6, HIGH);

  DOG.begin(6,0,0, 0, 1,DOGM128); 
  DOG.clear();
  DOG.createCanvas(128, 64, 0, 0);

  DOG.drawLine(0, 0, 127, 63);
  DOG.drawCircle(50, 30, 20, false);
  DOG.drawCircle(20, 20, 10, true);
  DOG.drawRect(60, 20, 20, 10, true);
  DOG.drawRect(80, 40, 10, 20, false);
  DOG.drawCross(90, 20, 10, 10);
	DOG.deleteCanvas();
}

void loop() {
  // put your main code here, to run repeatedly:

}
