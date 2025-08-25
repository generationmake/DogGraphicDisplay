/*
 * Copyright (c) 2019 by generationmake bernhard@generationmake.de
 * Arduino library for Electronic Assembly Dog Graphic Display DOGM132-5, DOGM128-6, DOGL128-6 and DOGS102-6 (controller ST7565 and UC1701)
 *
 * based on the work by:
 * Copyright (c) 2014 by ELECTRONIC ASSEMBLY <technik@lcd-module.de>
 * EA DOG Graphic (ST7565R) software library for arduino.
 * (see git log for modifications)
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1,
 * as published by the Free Software Foundation.
 */

#include <Arduino.h>
#include <SPI.h>
#if defined(ARDUINO_ARCH_AVR)
#include <avr/pgmspace.h>
#endif

#include "DogGraphicDisplay.h"

#define INITLEN 14
byte init_DOGM128[INITLEN] = {0x40, 0xA1, 0xC0, 0xA6, 0xA2, 0x2F, 0xF8, 0x00, 0x27, 0x81, 0x16, 0xAC, 0x00, 0xAF};
byte init_DOGL128[INITLEN] = {0x40, 0xA1, 0xC0, 0xA6, 0xA2, 0x2F, 0xF8, 0x00, 0x27, 0x81, 0x10, 0xAC, 0x00, 0xAF};
byte init_DOGM132[INITLEN] = {0x40, 0xA1, 0xC0, 0xA6, 0xA2, 0x2F, 0xF8, 0x00, 0x23, 0x81, 0x1F, 0xAC, 0x00, 0xAF};
#define INITLEN_DOGS102 13
byte init_DOGS102[INITLEN_DOGS102] = {0x40, 0xA1, 0xC0, 0xA4, 0xA6, 0xA2, 0x2F, 0x27, 0x81, 0x10, 0xFA, 0x90, 0xAF};

//----------------------------------------------------public Functions----------------------------------------------------
//Please use these functions in your sketch

/*-----------------------------
constructor for class, not needed by Arduino but for complete class. does not do anything.
*/
DogGraphicDisplay::DogGraphicDisplay()
{
}

/*-----------------------------
destructor for class, not needed by Arduino but for complete class. Calls Arduino end function
*/
DogGraphicDisplay::~DogGraphicDisplay()
{
  end();
}

/*-----------------------------
Arduino begin function. Forward data to initialize function and initialize canvas fullscreen
*/
void DogGraphicDisplay::begin(byte p_cs, byte p_si, byte p_clk, byte p_a0, byte p_res, byte type)
{
  initialize(p_cs, p_si, p_clk, p_a0, p_res, type);
}

/*----------------------------
Func: Arduino begin function with Hardware SPI
Desc: Initializes SPI Hardware and DOG Displays
Vars: Spi-Port, CS-Pin, A0-Pin (high=data, low=command), p_res = Reset-Pin, type (1=EA DOGM128-6, 2=EA DOGL128-6)
------------------------------*/
void DogGraphicDisplay::begin(SPIClass *port, byte p_cs, byte p_a0, byte p_res, byte type)
{
  byte *ptr_init; //pointer to the correct init values
  top_view = false; //default = bottom view

//  DogGraphicDisplay::spi_port = port;
  DogGraphicDisplay::p_a0 = p_a0;
  pinMode(p_a0, OUTPUT);
  spi_initialize_h(port, p_cs); //init SPI to Mode 3

  //perform a Reset
  digitalWrite(p_res, LOW);
  pinMode(p_res, OUTPUT);
  delayMicroseconds(10);
  digitalWrite(p_res, HIGH);
  delay(1);

  //Init DOGM displays, depending on users choice
  ptr_init = init_DOGM128; //default pointer for wrong parameters
  if(type == DOGM128) ptr_init = init_DOGM128;
  else if(type == DOGL128) ptr_init = init_DOGL128;
  else if(type == DOGM132) ptr_init = init_DOGM132;
  else if(type == DOGS102) ptr_init = init_DOGS102;

  DogGraphicDisplay::type = type;

  digitalWrite(p_a0, LOW);  //init display
  if(type == DOGS102) spi_put(ptr_init, INITLEN_DOGS102);  // shorter init for DOGS102
  else spi_put(ptr_init, INITLEN);

  clear();
}

/*-----------------------------
Arduino end function. stop SPI if enabled and delete canvas memory area
*/
void DogGraphicDisplay::end()
{
  if(hardware)
    SPI.end();
  delete [] canvas;
}

/*----------------------------
Func: DOG-INIT
Desc: Initializes SPI Hardware/Software and DOG Displays
Vars: CS-Pin, MOSI-Pin, SCK-Pin (MOSI=SCK Hardware else Software), A0-Pin (high=data, low=command), p_res = Reset-Pin, type (1=EA DOGM128-6, 2=EA DOGL128-6)
------------------------------*/
void DogGraphicDisplay::initialize(byte p_cs, byte p_si, byte p_clk, byte p_a0, byte p_res, byte type)
{
  byte *ptr_init; //pointer to the correct init values
  top_view = false; //default = bottom view

  DogGraphicDisplay::p_a0 = p_a0;
  pinMode(p_a0, OUTPUT);
  spi_initialize(p_cs, p_si, p_clk); //init SPI to Mode 3

  //perform a Reset
  digitalWrite(p_res, LOW);
  pinMode(p_res, OUTPUT);
  delayMicroseconds(10);
  digitalWrite(p_res, HIGH);
  delay(1);

  //Init DOGM displays, depending on users choice
  ptr_init = init_DOGM128; //default pointer for wrong parameters
  if(type == DOGM128) ptr_init = init_DOGM128;
  else if(type == DOGL128) ptr_init = init_DOGL128;
  else if(type == DOGM132) ptr_init = init_DOGM132;
  else if(type == DOGS102) ptr_init = init_DOGS102;

  DogGraphicDisplay::type = type;

  digitalWrite(p_a0, LOW);  //init display
  if(type == DOGS102) spi_put(ptr_init, INITLEN_DOGS102);  // shorter init for DOGS102
  else spi_put(ptr_init, INITLEN);

  clear();
}

/*----------------------------
Func: clear_display
Desc: clears the entire DOG-Display
Vars: ---
------------------------------*/
void DogGraphicDisplay::clear(void)
{
  byte page, column;
  byte page_cnt = 8;

  if(type == DOGM132)
  {
    page_cnt = 4;
  }

  for(page = 0; page < page_cnt; page++) //Display has 8 pages
  {
    position(0,page);
    digitalWrite(p_cs, LOW);
    digitalWrite(p_a0, HIGH);

    for(column = 0; column < display_width(); column++) //clear the whole page line
      spi_out(0x00);

    digitalWrite(p_cs, HIGH);
  }
}

/*----------------------------
Func: contrast
Desc: sets contrast to the DOG-Display
Vars: byte contrast (0..63)
------------------------------*/
void DogGraphicDisplay::contrast(byte contr)
{
  command(0x81);  //double byte command
  command(contr&0x3F);  //contrast has only 6 bits
}

/*----------------------------
Func: view
Desc: ssets the display viewing direction
Vars: direction (top view 0xC8, bottom view (default) = 0xC0)
------------------------------*/
void DogGraphicDisplay::view(byte direction)
{
  if(direction == VIEW_TOP)
  {
    top_view = true;
    command(0xA0);
  }
  else
  {
    top_view = false;
    command(0xA1);
  }

  command(direction);

  clear(); //Clear screen, as old content is not usable (mirrored)
}

/*----------------------------
Func: all_pixel_on
Desc: sets all pixel of the display to on
Vars: state (false=show SRAM content, true=set all pixel to on)
------------------------------*/
void DogGraphicDisplay::all_pixel_on(bool state)
{
  if(state == false)
  {
    command(0xA4);  // normal mode
  }
  else
  {
    command(0xA5);  // all pixels on
  }
}

/*----------------------------
Func: inverse
Desc: inverse content of display
Vars: state (false=normal content, true=inverse content)
------------------------------*/
void DogGraphicDisplay::inverse(bool state)
{
  if(state == false)
  {
    command(0xA6);  // normal mode
  }
  else
  {
    command(0xA7);  // inverse mode
  }
}

/*----------------------------
Func: sleep
Desc: sends the display to sleep mode (on/off)
Vars: state (false=normal mode, true=sleep mode)
------------------------------*/
void DogGraphicDisplay::sleep(bool state)
{
  if(state == false)
  {
    command(0xAF);  // normal mode
  }
  else
  {
    command(0xAE);  // sleep mode
  }
}

/*----------------------------
Func: string
Desc: shows string with selected font on position
Vars: column (0..127/131), page(0..3/7),  font address in program memory, stringarray
------------------------------*/
void DogGraphicDisplay::string(int column, byte page, const byte *font_adress, const char *str)
{
  string(column, page, font_adress, str, ALIGN_LEFT, STYLE_NORMAL);
}

/*----------------------------
Func: string
Desc: shows string with selected font on position with align
Vars: column (0..127/131), page(0..3/7),  font address in program memory, stringarray
------------------------------*/
void DogGraphicDisplay::string(int column, byte page, const byte *font_adress, const char *str, byte align)
{
  string(column, page, font_adress, str, align, STYLE_NORMAL);
}

/*----------------------------
Func: string
Desc: shows string with selected font on position with align and style
Vars: column (0..127/131), page(0..3/7),  font address in program memory, stringarray, align, style
------------------------------*/
void DogGraphicDisplay::string(int column, byte page, const byte *font_adress, const char *str, byte align, byte style)
{
  unsigned int pos_array;  //Position of character data in memory array
  byte x, y, width_max,width_min;  //temporary column and page address, couloumn_cnt tand width_max are used to stay inside display area
  int column_cnt;  //temporary column and page address, couloumn_cnt tand width_max are used to stay inside display area
  byte start_code, last_code, width, page_height, bytes_p_char; //font information, needed for calculation
  const char *string;
  int stringwidth=0; // width of string in pixels

#if defined(ARDUINO_ARCH_AVR)
  start_code = pgm_read_byte(&font_adress[2]);  //get first defined character
  last_code = pgm_read_byte(&font_adress[3]);  //get last defined character
  width = pgm_read_byte(&font_adress[4]);  //width in pixel of one char
  page_height = pgm_read_byte(&font_adress[6]);  //page count per char
  bytes_p_char = pgm_read_byte(&font_adress[7]);  //bytes per char
#else
  start_code = font_adress[2];  //get first defined character
  last_code = font_adress[3];  //get last defined character
  width = font_adress[4];  //width in pixel of one char
  page_height = font_adress[6];  //page count per char
  bytes_p_char = font_adress[7];  //bytes per char
#endif

  string = str;  //temporary pointer to the beginning of the string to print
  while(*string != 0)
  {
    if((byte)*string < start_code || (byte)*string > last_code) //make sure data is valid
      string++;
    else
    {
      string++;
      stringwidth++;
    }
  }
  stringwidth*=width;

  if(type != DOGM132 && page_height + page > 8) //stay inside display area
    page_height = 8 - page;
  else  if(type == DOGM132 && page_height + page > 4)
    page_height = 4 - page;

  if(align==ALIGN_RIGHT)
  {
    if(column==0) column=display_width()-stringwidth;  //if column is 0 align string to the right border
    else column=column-stringwidth;
  }
  if(align==ALIGN_CENTER) column=(display_width()-stringwidth)/2;

  //The string is displayed character after character. If the font has more then one page,
  //the top page is printed first, then the next page and so on
  for(y = 0; y < page_height; y++)
  {
    if(style==STYLE_FULL || style==STYLE_FULL_INVERSE)
    {
      position(0, page+y); //set startpositon and page
      column_cnt=0;
      digitalWrite(p_a0, HIGH);
      digitalWrite(p_cs, LOW);
      while(column_cnt<column)  // fill columns until beginning of string
      {
        column_cnt++;
        if(style==STYLE_FULL_INVERSE) spi_out(0xFF);
        else spi_out(0);
      }
    }
    else if(column<0) position(0,page+y);
    else position(column, page+y); //set startpositon and page
    column_cnt = column; //store column for display last column check
    string = str; //temporary pointer to the beginning of the string to print
    digitalWrite(p_a0, HIGH);
    digitalWrite(p_cs, LOW);
    while(*string != 0)
    {
      if(column_cnt>display_width()) string++;
      else if(column_cnt+width<0)
      {
        string++;
        column_cnt+=width;
      }
      else if((byte)*string < start_code || (byte)*string > last_code) //make sure data is valid
        string++;
      else
      {
        //calculate position of ascii character in font array
        //bytes for header + (ascii - startcode) * bytes per char)
        pos_array = 8 + (unsigned int)(*string++ - start_code) * bytes_p_char;
        pos_array += y*width; //get the dot pattern for the part of the char to print

        if((column_cnt + width) > display_width()) //stay inside display area
          width_max = display_width()-column_cnt;
        else
          width_max = width;

        if(column_cnt<0) width_min=0-column_cnt;
        else width_min=0;

        for(x=width_min; x < width_max; x++) //print the whole string
        {
#if defined(ARDUINO_ARCH_AVR)
          if(style==STYLE_INVERSE || style==STYLE_FULL_INVERSE) spi_out(~pgm_read_byte(&font_adress[pos_array+x]));
          else spi_out(pgm_read_byte(&font_adress[pos_array+x]));
#else
          if(style==STYLE_INVERSE || style==STYLE_FULL_INVERSE) spi_out(~font_adress[pos_array+x]);
          else spi_out(font_adress[pos_array+x]);
#endif
          //spi_out(pgm_read_byte(&font_adress[pos_array+x])); //double width font (bold)
        }
        column_cnt+=width;
      }
    }
    if(style==STYLE_FULL || style==STYLE_FULL_INVERSE)
    {
      column_cnt=column+stringwidth;
      while(column_cnt<display_width())
      {
        column_cnt++;
        if(style==STYLE_FULL_INVERSE) spi_out(0xFF);
        else spi_out(0);
      }
    }
    digitalWrite(p_cs, HIGH);
  }
}

/*----------------------------
Func: rectangle
Desc: shows a pattern filled rectangle on the display
Vars: start and end column (0..127/131) and page(0..3/7), bit pattern
------------------------------*/
void DogGraphicDisplay::rectangle(byte start_column, byte start_page, byte end_column, byte end_page, byte pattern)
{
  byte x, y;

  if(end_column>display_width())  //stay inside display area
    end_column=display_width();
  if(type != DOGM132 && end_page > 7)
    end_page = 7;
  else if (type == DOGM132 && end_page > 3)
    end_page = 3;

  for(y=start_page; y<=end_page; y++)
  {
    position(start_column, y);
    digitalWrite(p_a0, HIGH);
    digitalWrite(p_cs, LOW);

    for(x=start_column; x<=end_column; x++)
      spi_out(pattern);

    digitalWrite(p_cs, HIGH);
  }
}

/*----------------------------
Func: picture
Desc: shows a BLH-picture on the display (see BitMapEdit EA LCD-Tools (http://www.lcd-module.de/support.html))
Vars: column (0..127/131) and page(0..3/7), program memory address of data
------------------------------*/
void DogGraphicDisplay::picture(byte column, byte page, const byte *pic_adress)
{
  byte c,p;
  unsigned int byte_cnt = 2;
  byte width,picture_width, page_cnt;

#if defined(ARDUINO_ARCH_AVR)
  picture_width = pgm_read_byte(&pic_adress[0]);
  page_cnt = (pgm_read_byte(&pic_adress[1]) + 7) / 8; //height in pages, add 7 and divide by 8 for getting the used pages (byte boundaries)
#else
  picture_width = pic_adress[0];
  page_cnt = (pic_adress[1] + 7) / 8; //height in pages, add 7 and divide by 8 for getting the used pages (byte boundaries)
#endif

  if((picture_width + column) > display_width()) //stay inside display area
    width = display_width() - column;
  else width=picture_width;

  if(type != DOGM132 && page_cnt + page > 8)
    page_cnt = 8 - page;
  else if(type == DOGM132 && page_cnt + page > 4)
    page_cnt = 4 - page;

  for(p=0; p<page_cnt; p++)
  {
    byte_cnt=2+p*picture_width; // set byte counter to the correct start position in case that picture does not fit on screen
    position(column, page + p);
    digitalWrite(p_a0, HIGH);
    digitalWrite(p_cs, LOW);

    for(c=0; c<width; c++)
#if defined(ARDUINO_ARCH_AVR)
      spi_out(pgm_read_byte(&pic_adress[byte_cnt++]));
#else
      spi_out(pic_adress[byte_cnt++]);
#endif

    digitalWrite(p_cs, HIGH);
  }
}

/*----------------------------
Func: picture with style
Desc: shows a BLH-picture on the display (see BitMapEdit EA LCD-Tools (http://www.lcd-module.de/support.html))
Vars: column (0..127/131) and page(0..3/7), program memory address of data
------------------------------*/
void DogGraphicDisplay::picture(byte column, byte page, const byte *pic_adress, byte style)
{
  byte c,p;
  unsigned int byte_cnt = 2;
  byte width,picture_width, page_cnt;

#if defined(ARDUINO_ARCH_AVR)
  picture_width = pgm_read_byte(&pic_adress[0]);
  page_cnt = (pgm_read_byte(&pic_adress[1]) + 7) / 8; //height in pages, add 7 and divide by 8 for getting the used pages (byte boundaries)
#else
  picture_width = pic_adress[0];
  page_cnt = (pic_adress[1] + 7) / 8; //height in pages, add 7 and divide by 8 for getting the used pages (byte boundaries)
#endif

  if((picture_width + column) > display_width()) //stay inside display area
    width = display_width() - column;
  else width=picture_width;

  if(type != DOGM132 && page_cnt + page > 8)
    page_cnt = 8 - page;
  else if(type == DOGM132 && page_cnt + page > 4)
    page_cnt = 4 - page;

  for(p=0; p<page_cnt; p++)
  {
    byte_cnt=2+p*picture_width; // set byte counter to the correct start position in case that picture does not fit on screen
    position(column, page + p);
    digitalWrite(p_a0, HIGH);
    digitalWrite(p_cs, LOW);

    for(c=0; c<width; c++)
#if defined(ARDUINO_ARCH_AVR)
      if(style==STYLE_INVERSE || style==STYLE_FULL_INVERSE) spi_out(~pgm_read_byte(&pic_adress[byte_cnt++]));
      else spi_out(pgm_read_byte(&pic_adress[byte_cnt++]));
#else
      if(style==STYLE_INVERSE || style==STYLE_FULL_INVERSE) spi_out(~pic_adress[byte_cnt++]);
      else spi_out(pic_adress[byte_cnt++]);
#endif

    digitalWrite(p_cs, HIGH);
  }
}

/*----------------------------
Func: display_width
Desc: returns the width of the display
Vars: none
------------------------------*/
byte DogGraphicDisplay::display_width (void)
{
  byte column_total=128;
  if(type == DOGM132)
    column_total = 132;
  if(type == DOGS102)  // define different parameters for DOGS102
    column_total = 102;

  return column_total;
}
/*----------------------------
Func: page_cnt
Desc: returns the page count of the display
Vars: none
------------------------------*/
byte DogGraphicDisplay::page_cnt(void)
{
  byte page_cnt = 8;

  if(type == DOGM132)
  {
    page_cnt = 4;
  }
  return page_cnt;
}

/*----------------------------
Func: createCanvas
Desc: creates Canvas
Vars: canvas size and point of upper left corner, y-direction page aligned
------------------------------*/
void DogGraphicDisplay::createCanvas(byte canvasSizeX, byte canvasSizeY, int upperLeftX, int upperLeftY)
{
  createCanvas(canvasSizeX, canvasSizeY, upperLeftX, upperLeftY, 0);
}

/*----------------------------
Func: createCanvas
Desc: creates Canvas
Vars: canvas size and point of upper left corner, y-direction page aligned, drawMode (0=direct to display, other=buffered)
------------------------------*/
void DogGraphicDisplay::createCanvas(byte canvasSizeX, byte canvasSizeY, int upperLeftX, int upperLeftY, byte drawMode)
{
  this->canvasSizeX = canvasSizeX;
  this->canvasSizeY = canvasSizeY;
  this->canvasUpperLeftX = upperLeftX;
  this->canvasUpperLeftY = upperLeftY;
  this->drawMode = drawMode;

  byte rest = this->canvasSizeY % 8;

  if(rest > 0)
  {
    for(int n = 1; n <= 8; n++)
    {
      int next = n * 8;
      if(this->canvasSizeY < next)
      {
        this->canvasSizeY = next;
        break;
      }
    }
  }

  canvasPages = this->canvasSizeY / 8;
  canvas = new byte[canvasSizeX * canvasPages];


  for(int x = 0; x < canvasSizeX; x++)
  {
    for(int page = 0; page < canvasPages; page ++)
    {
      canvas[page * canvasSizeX + x] = 0;
    }
  }
}

/*----------------------------
Func: deleteCanvas
Desc: deletes Canvas so memory is free again
------------------------------*/
void DogGraphicDisplay::deleteCanvas()
{
  delete[] canvas;
}

/*----------------------------
Func: setPixel
Desc: set single pixel value
Vars: x, y coordinates, value(true = black, false = white
------------------------------*/
void DogGraphicDisplay::setPixel(int x, int y, bool value)
{
//  x += canvasUpperLeftX;
//  y += canvasUpperLeftY;

  if(x < canvasSizeX && y < canvasSizeY && x >= 0 && y >= 0) // check if pixel is within canvas
  {

    byte page = (y * canvasPages) / canvasSizeY;
    y = y - 8 * page;
    if(value)
    {
      canvas[page * canvasSizeX + x] |= (1<<y);
    }
    else
    {
      canvas[page * canvasSizeX + x] &= ~(1<<y);
    }

    if(drawMode==0)
    {
      if((x+canvasUpperLeftX)>=0&&(x+canvasUpperLeftX)<display_width()&&(page+canvasUpperLeftY)>=0&&(page+canvasUpperLeftY)<page_cnt())  // check if pixel is within display
      {
        rectangle(x+canvasUpperLeftX, page+canvasUpperLeftY, x+canvasUpperLeftX, page+canvasUpperLeftY, canvas[page * canvasSizeX + x]);
      }
    }
  }
}

/*----------------------------
Func: drawLine
Desc: draw line on display
Vars: start and end coordinates
------------------------------*/
void DogGraphicDisplay::drawLine(int x0, int y0, int x1, int y1)
{
  int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
  int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1;
  int err = (dx>dy ? dx : -dy)/2, e2;

  for(;;){
    setPixel(x0,y0, true);
    if (x0==x1 && y0==y1) break;
    e2 = err;
    if (e2 >-dx) { err -= dy; x0 += sx; }
    if (e2 < dy) { err += dx; y0 += sy; }
  }
}

/*----------------------------
Func: drawArrow
Desc: draw arrow on display
Vars: start and end coordinates, the head is at the end coordinates
------------------------------*/
void DogGraphicDisplay::drawArrow(int x0, int y0, int x1, int y1)
{
  drawLine(x0,y0,x1,y1);
  drawLine(x1-(x1-x0)/6-(y1-y0)/6,y1-(y1-y0)/6+(x1-x0)/6,x1,y1);
  drawLine(x1-(x1-x0)/6+(y1-y0)/6,y1-(y1-y0)/6-(x1-x0)/6,x1,y1);
}

/*----------------------------
Func: drawCircle
Desc: draw circle on display
Vars: center coordinates, radius, fill( true = filled, false = not filled )
------------------------------*/
void DogGraphicDisplay::drawCircle(int x0, int y0, int r, bool fill)
{
  int x = r;
  int y = 0;
  int err = 0;

  while (x >= y)
  {
    if(!fill)
    {
      setPixel(x0 + x, y0 + y, true);
      setPixel(x0 + y, y0 + x, true);
      setPixel(x0 - y, y0 + x, true);
      setPixel(x0 - x, y0 + y, true);
      setPixel(x0 - x, y0 - y, true);
      setPixel(x0 - y, y0 - x, true);
      setPixel(x0 + y, y0 - x, true);
      setPixel(x0 + x, y0 - y, true);
    }
    else
    {
      drawLine(x0 + x, y0 + y, x0 - x, y0 + y);
      drawLine(x0 + y, y0 + x, x0 - y, y0 + x);
      drawLine(x0 - x, y0 - y, x0 + x, y0 - y);
      drawLine(x0 - y, y0 - x, x0 + y, y0 - x);
    }

    if (err <= 0)
    {
      y += 1;
      err += 2*y + 1;
    }

    if (err > 0)
    {
      x -= 1;
      err -= 2*x + 1;
    }
  }
}

/*----------------------------
Func: drawRect
Desc: draw rectangle on display
Vars: coordinates of upper left corner, width and height, fill( true = filled, false = not filled )
------------------------------*/
void DogGraphicDisplay::drawRect(int x0, int y0, int width, int height, bool fill)
{
  if(!fill)
  {
    drawLine(x0, y0, x0 + width, y0);
    drawLine(x0, y0 + height, x0 + width, y0 + height);
    drawLine(x0, y0, x0, y0 + height);
    drawLine(x0 + width, y0, x0 + width, y0 + height);
  }
  else
  {
    for(int y = y0; y <= y0 + height; y++)
    {
      drawLine(x0, y, x0 + width, y);
    }
  }
}

/*----------------------------
Func: drawCross
Desc: draw X on display
Vars: center coordinates, width and height
------------------------------*/
void DogGraphicDisplay::drawCross(int x0, int y0, int width, int height)
{
  drawLine(x0 - width, y0 - height, x0 + width, y0 + height);
  drawLine(x0 - width, y0 + height, x0 + width, y0 - height);
}

/*----------------------------
Func: clearCanvas
Desc: sets all pixel of the canvas to 0
Vars: none
------------------------------*/
void DogGraphicDisplay::clearCanvas(void)
{
  for(int x = 0; x < canvasSizeX; x++)
  {
    for(int y = 0; y < canvasSizeY; y++)
    {
      setPixel(x,y,0);
    }
  }
}

/*----------------------------
Func: flushCanvas
Desc: sends all pixel of the canvas to the display
Vars: coordinates of upper left corner
------------------------------*/
void DogGraphicDisplay::flushCanvas(int upperLeftX, int upperLeftY)
{
  this->canvasUpperLeftX = upperLeftX;
  this->canvasUpperLeftY = upperLeftY;
  flushCanvas();
}

/*----------------------------
Func: flushCanvas
Desc: sends all pixel of the canvas to the display
Vars: none
------------------------------*/
void DogGraphicDisplay::flushCanvas(void)
{
  for(int page = 0; page < canvasPages; page++)
  {
    if((page+canvasUpperLeftY)>=0&&(page+canvasUpperLeftY)<page_cnt())  // check if page is within display
    {
      int x=0;
      if(canvasUpperLeftX>=0) position(canvasUpperLeftX, page+canvasUpperLeftY);
      else
      {
        position(0, page+canvasUpperLeftY);
        x=-canvasUpperLeftX;
      }
      digitalWrite(p_a0, HIGH);
      digitalWrite(p_cs, LOW);

      for( ; ((x < canvasSizeX)&&((x+canvasUpperLeftX)<display_width())); x++)  // also check if x is within display
      {
        spi_out(canvas[page * canvasSizeX + x]);
      }

      digitalWrite(p_cs, HIGH);
    }
  }
}

//----------------------------------------------------private Functions----------------------------------------------------
//normally you don't need those functions in your sketch

/*----------------------------
Func: position
Desc: sets write pointer in DOG-Display
Vars: column (0..127/131), page(0..3/7)
------------------------------*/
void DogGraphicDisplay::position(byte column, byte page)
{
  if(top_view && type != DOGM132)
    column += 4;

  command(0x10 + (column>>4)); //MSB address column
  command(0x00 + (column&0x0F)); //LSB address column
  command(0xB0 + (page&0x0F)); //address page
}

/*----------------------------
Func: command
Desc: Sends a command to the DOG-Display
Vars: data
------------------------------*/
void DogGraphicDisplay::command(byte dat)
{
  digitalWrite(p_a0, LOW);
  spi_put_byte(dat);
}

/*----------------------------
Func: data
Desc: Sends data to the DOG-Display
Vars: data
------------------------------*/
void DogGraphicDisplay::data(byte dat)
{
  digitalWrite(p_a0, HIGH);
  spi_put_byte(dat);
}

/*----------------------------
Func: spi_initialize
Desc: Initializes SPI Hardware/Software
Vars: CS-Pin, MOSI-Pin, SCK-Pin (MOSI=SCK Hardware else Software)
------------------------------*/
void DogGraphicDisplay::spi_initialize(byte cs, byte si, byte clk)
{
  //Set pin Configuration
  p_cs = cs;

  if(si == clk)
  {
    hardware = true;
    p_si = MOSI;
    p_clk = SCK;
    DogGraphicDisplay::spi_port = &SPI;
  }
  else
  {
    hardware = false;
    p_si = si;
    p_clk = clk;
  }

  // Set CS to deselct slaves
  digitalWrite(p_cs, HIGH);
  pinMode(p_cs, OUTPUT);

  // Set Data pin as output
  pinMode(p_si, OUTPUT);

  // Set SPI-Mode 3: CLK idle high, rising edge, MSB first
  digitalWrite(p_clk, HIGH);
  pinMode(p_clk, OUTPUT);
  if(hardware)
  {
    SPI.begin();
    SPI.beginTransaction(SPISettings(10*1000*1000, MSBFIRST, SPI_MODE3)); /* SPI CLK = 10 MHz */
  }
}

/*----------------------------
Func: spi_initialize with hardware spi
Desc: Initializes SPI Hardware
Vars: Port, CS-Pin
------------------------------*/
void DogGraphicDisplay::spi_initialize_h(SPIClass *port, byte cs)
{
  //Set pin Configuration
  p_cs = cs;
  DogGraphicDisplay::spi_port = port;
  hardware = true;

  // Set CS to deselct slaves
  digitalWrite(p_cs, HIGH);
  pinMode(p_cs, OUTPUT);

  spi_port->begin();
  spi_port->beginTransaction(SPISettings(10*1000*1000, MSBFIRST, SPI_MODE3)); /* SPI CLK = 10 MHz */
}

/*----------------------------
Func: spi_put_byte
Desc: Sends one Byte using CS
Vars: data
------------------------------*/
void DogGraphicDisplay::spi_put_byte(byte dat)
{
  digitalWrite(p_cs, LOW);
  spi_out(dat);
  digitalWrite(p_cs, HIGH);
}

/*----------------------------
Func: spi_put
Desc: Sends bytes using CS
Vars: ptr to data and len
------------------------------*/
void DogGraphicDisplay::spi_put(byte *dat, int len)
{
  digitalWrite(p_cs, LOW);
  do
  {
    spi_out(*dat++);
  }while(--len);

  digitalWrite(p_cs, HIGH);
}

/*----------------------------
Func: spi_out
Desc: Sends one Byte, no CS
Vars: data
------------------------------*/
void DogGraphicDisplay::spi_out(byte dat)
{
  byte i = 8;
  if(hardware)
  {
    spi_port->transfer(dat);
  }
  else
  {
    do
    {
      if(dat & 0x80)
        digitalWrite(p_si, HIGH);
      else
        digitalWrite(p_si, LOW);
      digitalWrite(p_clk, LOW);
      dat <<= 1;
      digitalWrite(p_clk, HIGH);
    }while(--i);
  }
}
