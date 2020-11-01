/******************************************************************************
  Draws two turning circles with arrows on the display

  Original Creation Date: Nov. 1, 2020

  This code is Beerware; if you see me at the local,
  and you've found our code helpful, please buy us a round!

  Tested with ArduiHMI v1.3 and Arduino MKRZERO

  Distributed as-is; no warranty is given.
******************************************************************************/

#include <DogGraphicDisplay.h>
#include "dense_numbers_8.h"

DogGraphicDisplay DOG;

void setup() {
  pinMode(A6, OUTPUT);   // set backlight pin to output
  digitalWrite(A6, HIGH);

  DOG.begin(6,0,0, 0, 1,DOGM128);
  DOG.clear();
  DOG.createCanvas(128, 64, 0, 0, 1);  // Canvas in buffered mode

}

void loop() {
  // put your main code here, to run repeatedly:
  const int circle1_x=32;
  const int circle1_y=32;
  const int circle1_radius=31;
  const int circle2_x=96;
  const int circle2_y=20;
  const int circle2_radius=20;
  static float degree1=0.0;
  static float degree2=0.0;

  degree1+=1.0;
  if(degree1>=360.0) degree1-=360;
  degree2+=2.7;
  if(degree2>=360.0) degree2-=360;

  float diff1_x=(circle1_radius-1)*sin(degree1*DEG_TO_RAD);
  float diff1_y=(circle1_radius-1)*cos(degree1*DEG_TO_RAD);
  float diff2_x=(circle2_radius-1)*sin(degree2*DEG_TO_RAD);
  float diff2_y=(circle2_radius-1)*cos(degree2*DEG_TO_RAD);

  DOG.clearCanvas();
  DOG.drawCircle(circle1_x, circle1_y, circle1_radius, false);
  DOG.drawArrow(circle1_x, circle1_y, circle1_x+diff1_x, circle1_y-diff1_y);
  DOG.drawCircle(circle2_x, circle2_y, circle2_radius, false);
  DOG.drawArrow(circle2_x-diff2_x, circle2_y+diff2_y, circle2_x+diff2_x, circle2_y-diff2_y);
  DOG.flushCanvas();
  String degree1_str(degree1);
  DOG.string(18,5,DENSE_NUMBERS_8,degree1_str.c_str());
  String degree2_str(degree2);
  DOG.string(85,6,DENSE_NUMBERS_8,degree2_str.c_str());
  delay(100);

}
