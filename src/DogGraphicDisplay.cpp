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

/*----------------------------
Func: DOG-INIT
Desc: Initializes SPI Hardware/Software and DOG Displays
Vars: CS-Pin, MOSI-Pin, SCK-Pin (MOSI=SCK Hardware else Software), A0-Pin (high=data, low=command), p_res = Reset-Pin, type (1=EA DOGM128-6, 2=EA DOGL128-6)
------------------------------*/
void dogGraphicDisplay::initialize(byte p_cs, byte p_si, byte p_clk, byte p_a0, byte p_res, byte type) 
{
	byte *ptr_init; //pointer to the correct init values
	top_view = false; //default = bottom view

	dogGraphicDisplay::p_a0 = p_a0;
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
	if(type == DOGM128) 		ptr_init = init_DOGM128;
	else if(type == DOGL128) 	ptr_init = init_DOGL128;
	else if(type == DOGM132) 	ptr_init = init_DOGM132;
	else if(type == DOGS102) 	ptr_init = init_DOGS102;
	
	dogGraphicDisplay::type = type;

	digitalWrite(p_a0, LOW); //init display
	if(type == DOGS102) spi_put(ptr_init, INITLEN_DOGS102);	// shorter init for DOGS102
	else spi_put(ptr_init, INITLEN);

	clear();
}

/*----------------------------
Func: clear_display
Desc: clears the entire DOG-Display
Vars: ---
------------------------------*/
void dogGraphicDisplay::clear(void) 
{
	byte page, column;
	byte page_cnt = 8, column_cnt = 128;
	
	if(type == DOGM132)
	{
		page_cnt = 4;
		column_cnt = 132;
	}
	if(type == DOGS102)		// define different parameters for DOGS102
	{
		page_cnt = 8;
		column_cnt = 102;
	}
	
	for(page = 0; page < page_cnt; page++) //Display has 8 pages
	{
		position(0,page);
		digitalWrite(p_cs, LOW);
		digitalWrite(p_a0, HIGH);
		
		for(column = 0; column < column_cnt; column++) //clear the whole page line
			spi_out(0x00);
		
		digitalWrite(p_cs, HIGH);
	}
}

/*----------------------------
Func: contrast
Desc: sets contrast to the DOG-Display
Vars: byte contrast (0..63)
------------------------------*/
void dogGraphicDisplay::contrast(byte contr) 
{
	command(0x81);   		//double byte command
	command(contr&0x3F);	//contrast has only 6 bits
}

/*----------------------------
Func: view
Desc: ssets the display viewing direction
Vars: direction (top view 0xC8, bottom view (default) = 0xC0)
------------------------------*/
void dogGraphicDisplay::view(byte direction)  
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
void dogGraphicDisplay::all_pixel_on(bool state)  
{
	if(state == false)
	{
		command(0xA4);	// normal mode
	}
	else
	{
		command(0xA5);	// all pixels on
	}
}

/*----------------------------
Func: inverse
Desc: inverse content of display
Vars: state (false=normal content, true=inverse content)
------------------------------*/
void dogGraphicDisplay::inverse(bool state)  
{
	if(state == false)
	{
		command(0xA6);	// normal mode
	}
	else
	{
		command(0xA7);	// inverse mode
	}
}

/*----------------------------
Func: sleep
Desc: sends the display to sleep mode (on/off)
Vars: state (false=normal mode, true=sleep mode)
------------------------------*/
void dogGraphicDisplay::sleep(bool state)  
{
	if(state == false)
	{
		command(0xAF);	// normal mode
	}
	else
	{
		command(0xAE);	// sleep mode
	}
}

/*----------------------------
Func: string
Desc: shows string with selected font on position
Vars: column (0..127/131), page(0..3/7),  font adress in programm memory, stringarray
------------------------------*/
void dogGraphicDisplay::string(byte column, byte page, const byte *font_adress, const char *str)
{
	unsigned int pos_array; 										//Postion of character data in memory array
	byte x, y, column_cnt, width_max;								//temporary column and page adress, couloumn_cnt tand width_max are used to stay inside display area
	byte start_code, last_code, width, page_height, bytes_p_char;	//font information, needed for calculation
	const char *string;

	
	
#if defined(ARDUINO_ARCH_AVR)
	start_code 	 = pgm_read_byte(&font_adress[2]);  //get first defined character
	last_code	 = pgm_read_byte(&font_adress[3]);  //get last defined character
	width		 = pgm_read_byte(&font_adress[4]);  //width in pixel of one char
	page_height  = pgm_read_byte(&font_adress[6]);  //page count per char
	bytes_p_char = pgm_read_byte(&font_adress[7]);  //bytes per char
#else
	start_code 	 = font_adress[2];  //get first defined character
	last_code	 = font_adress[3];  //get last defined character
	width		 = font_adress[4];  //width in pixel of one char
	page_height  = font_adress[6];  //page count per char
	bytes_p_char = font_adress[7];  //bytes per char
#endif
  
  if(type != DOGM132 && page_height + page > 8) //stay inside display area
		page_height = 8 - page;
  else  if(type == DOGM132 && page_height + page > 4)
    page_height = 4 - page;
  
	
	//The string is displayed character after character. If the font has more then one page, 
	//the top page is printed first, then the next page and so on
	for(y = 0; y < page_height; y++)
	{
		position(column, page+y); //set startpositon and page
		column_cnt = column; //store column for display last column check
		string = str;             //temporary pointer to the beginning of the string to print
		digitalWrite(p_a0, HIGH);
		digitalWrite(p_cs, LOW);
		while(*string != 0)
		{	
			if((byte)*string < start_code || (byte)*string > last_code) //make sure data is valid
				string++;
			else
			{							
				//calculate positon of ascii character in font array
				//bytes for header + (ascii - startcode) * bytes per char)
				pos_array = 8 + (unsigned int)(*string++ - start_code) * bytes_p_char;
				pos_array += y*width; //get the dot pattern for the part of the char to print
        
        if(type != DOGM132 && type != DOGS102 && column_cnt + width > 128) //stay inside display area
		width_max = 128-column_cnt;
        else if(type == DOGM132 && column_cnt + width > 132)
           width_max = 132-column_cnt;
        else if(type == DOGS102 && column_cnt + width > 102)
           width_max = 102-column_cnt;
				else
					width_max = width;
          
				for(x=0; x < width_max; x++) //print the whole string
				{
#if defined(ARDUINO_ARCH_AVR)
					spi_out(pgm_read_byte(&font_adress[pos_array+x]));
#else
					spi_out(font_adress[pos_array+x]);
#endif
					//spi_out(pgm_read_byte(&font_adress[pos_array+x])); //double width font (bold)
				}
			}
		}
		digitalWrite(p_cs, HIGH);
	}
}

/*----------------------------
Func: string
Desc: shows string with selected font on position with align
Vars: column (0..127/131), page(0..3/7),  font adress in programm memory, stringarray
------------------------------*/
void dogGraphicDisplay::string(byte column, byte page, const byte *font_adress, const char *str, byte align)
{
	unsigned int pos_array; 		//Postion of character data in memory array
	byte x, y, column_cnt, width_max;	//temporary column and page adress, couloumn_cnt tand width_max are used to stay inside display area
	byte start_code, last_code, width, page_height, bytes_p_char;	//font information, needed for calculation
	const char *string;
	int stringwidth=0; // width of string in pixels
	int column_total = 128;

#if defined(ARDUINO_ARCH_AVR)
	start_code 	 = pgm_read_byte(&font_adress[2]);  //get first defined character
	last_code	 = pgm_read_byte(&font_adress[3]);  //get last defined character
	width		 = pgm_read_byte(&font_adress[4]);  //width in pixel of one char
	page_height  = pgm_read_byte(&font_adress[6]);  //page count per char
	bytes_p_char = pgm_read_byte(&font_adress[7]);  //bytes per char
#else
	start_code 	 = font_adress[2];  //get first defined character
	last_code	 = font_adress[3];  //get last defined character
	width		 = font_adress[4];  //width in pixel of one char
	page_height  = font_adress[6];  //page count per char
	bytes_p_char = font_adress[7];  //bytes per char
#endif

	string = str;             //temporary pointer to the beginning of the string to print
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

	
	if(type == DOGM132)
		column_total = 132;
	if(type == DOGS102)		// define different parameters for DOGS102
		column_total = 102;

	if(align==ALIGN_RIGHT) 
	{
		if(column==0) column=column_total-stringwidth;
		else column=column-stringwidth;
	}
	if(align==ALIGN_CENTER) column=(column_total-stringwidth)/2;
  	
	//The string is displayed character after character. If the font has more then one page, 
	//the top page is printed first, then the next page and so on
	for(y = 0; y < page_height; y++)
	{
		position(column, page+y); //set startpositon and page
		column_cnt = column; //store column for display last column check
		string = str;             //temporary pointer to the beginning of the string to print
		digitalWrite(p_a0, HIGH);
		digitalWrite(p_cs, LOW);
		while(*string != 0)
		{	
			if((byte)*string < start_code || (byte)*string > last_code) //make sure data is valid
				string++;
			else
			{							
				//calculate positon of ascii character in font array
				//bytes for header + (ascii - startcode) * bytes per char)
				pos_array = 8 + (unsigned int)(*string++ - start_code) * bytes_p_char;
				pos_array += y*width; //get the dot pattern for the part of the char to print
        
        if(type != DOGM132 && type != DOGS102 && column_cnt + width > 128) //stay inside display area
		width_max = 128-column_cnt;
        else if(type == DOGM132 && column_cnt + width > 132)
           width_max = 132-column_cnt;
        else if(type == DOGS102 && column_cnt + width > 102)
           width_max = 102-column_cnt;
				else
					width_max = width;
          
				for(x=0; x < width_max; x++) //print the whole string
				{
#if defined(ARDUINO_ARCH_AVR)
					spi_out(pgm_read_byte(&font_adress[pos_array+x]));
#else
					spi_out(font_adress[pos_array+x]);
#endif
					//spi_out(pgm_read_byte(&font_adress[pos_array+x])); //double width font (bold)
				}
			}
		}
		digitalWrite(p_cs, HIGH);
	}
}

/*----------------------------
Func: stringx with offset
Desc: shows string with selected font on position
Vars: column (0..127/131), page(0..3/7),  font adress in programm memory, stringarray
------------------------------*/
void dogGraphicDisplay::stringx(byte column, byte page, int offset, const byte *font_adress, const char *str)
{
	unsigned int pos_array; 										//Postion of character data in memory array
	byte x, y, width_max,width_min;								//temporary column and page adress, couloumn_cnt tand width_max are used to stay inside display area
	int column_cnt,columnx;								//temporary column and page adress, couloumn_cnt tand width_max are used to stay inside display area
	byte start_code, last_code, width, page_height, bytes_p_char;	//font information, needed for calculation
	const char *string;

	
	
#if defined(ARDUINO_ARCH_AVR)
	start_code 	 = pgm_read_byte(&font_adress[2]);  //get first defined character
	last_code	 = pgm_read_byte(&font_adress[3]);  //get last defined character
	width		 = pgm_read_byte(&font_adress[4]);  //width in pixel of one char
	page_height  = pgm_read_byte(&font_adress[6]);  //page count per char
	bytes_p_char = pgm_read_byte(&font_adress[7]);  //bytes per char
#else
	start_code 	 = font_adress[2];  //get first defined character
	last_code	 = font_adress[3];  //get last defined character
	width		 = font_adress[4];  //width in pixel of one char
	page_height  = font_adress[6];  //page count per char
	bytes_p_char = font_adress[7];  //bytes per char
#endif
  
  if(type != DOGM132 && page_height + page > 8) //stay inside display area
		page_height = 8 - page;
  else  if(type == DOGM132 && page_height + page > 4)
    page_height = 4 - page;

columnx=column+offset;
  
	
	//The string is displayed character after character. If the font has more then one page, 
	//the top page is printed first, then the next page and so on
	for(y = 0; y < page_height; y++)
	{
		if(columnx<0) position(0, page+y); //set startpositon and page
		else position(columnx, page+y); //set startpositon and page
		column_cnt = columnx; //store column for display last column check
		string = str;             //temporary pointer to the beginning of the string to print
		digitalWrite(p_a0, HIGH);
		digitalWrite(p_cs, LOW);
		while(*string != 0)
		{	
			if(column_cnt>128) string++;
			else if(column_cnt+width<0) 
			{
				string++;
				column_cnt+=width;
				
			}
			else if((byte)*string < start_code || (byte)*string > last_code) //make sure data is valid
				string++;
			else
			{							
				//calculate positon of ascii character in font array
				//bytes for header + (ascii - startcode) * bytes per char)
				pos_array = 8 + (unsigned int)(*string++ - start_code) * bytes_p_char;
				pos_array += y*width; //get the dot pattern for the part of the char to print
        
        if(type != DOGM132 && type != DOGS102 && column_cnt + width > 128) //stay inside display area
					width_max = 128-column_cnt;
        else if(type == DOGM132 && column_cnt + width > 132)
           width_max = 132-column_cnt;
        else if(type == DOGS102 && column_cnt + width > 102)
           width_max = 102-column_cnt;
				else
					width_max = width;
				if(column_cnt<0) width_min=0-column_cnt;
				else width_min=0;
          
				for(x=width_min; x < width_max; x++) //print the whole string
				{
#if defined(ARDUINO_ARCH_AVR)
					spi_out(pgm_read_byte(&font_adress[pos_array+x]));
#else
					spi_out(font_adress[pos_array+x]);
#endif
					//spi_out(pgm_read_byte(&font_adress[pos_array+x])); //double width font (bold)
				}
				column_cnt+=width;
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
void dogGraphicDisplay::rectangle(byte start_column, byte start_page, byte end_column, byte end_page, byte pattern)  
{
	byte x, y;
  
  if(type != DOGM132 && type != DOGS102 && end_column > 128) //stay inside display area
		end_column = 128;
  else if(type == DOGM132 && end_column > 132)
     end_column = 132;
  else if(type == DOGS102 && end_column > 102)
     end_column = 102;
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
Vars: column (0..127/131) and page(0..3/7), program memory adress of data
------------------------------*/
void dogGraphicDisplay::picture(byte column, byte page, const byte *pic_adress)  
{
	byte c,p;
	unsigned int byte_cnt = 2;
	byte width, page_cnt;
		
#if defined(ARDUINO_ARCH_AVR)
	width = pgm_read_byte(&pic_adress[0]);
	page_cnt = (pgm_read_byte(&pic_adress[1]) + 7) / 8; //height in pages, add 7 and divide by 8 for getting the used pages (byte boundaries)
#else
	width = pic_adress[0];
	page_cnt = (pic_adress[1] + 7) / 8; //height in pages, add 7 and divide by 8 for getting the used pages (byte boundaries)
#endif
	     
  if(width + column > 128 && type != DOGM132 && type != DOGS102) //stay inside display area
		width = 128 - column;
  else if(width + column > 132 && type == DOGM132)
    width = 132 - column;
  else if(width + column > 102 && type == DOGS102)
    width = 102 - column;
  
  if(type != DOGM132 && page_cnt + page > 8)
		page_cnt = 8 - page;
  else if(type == DOGM132 && page_cnt + page > 4)
    page_cnt = 4 - page;
	
	for(p=0; p<page_cnt; p++)
	{
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

//----------------------------------------------------private Functions----------------------------------------------------
//normally you don't need those functions in your sketch

/*----------------------------
Func: position
Desc: sets write pointer in DOG-Display
Vars: column (0..127/131), page(0..3/7)
------------------------------*/
void dogGraphicDisplay::position(byte column, byte page)  
{
	if(top_view && type != DOGM132)
		column += 4;
		
	command(0x10 + (column>>4)); 	//MSB adress column
	command(0x00 + (column&0x0F));	//LSB adress column
	command(0xB0 + (page&0x0F)); 	//adress page	
}

/*----------------------------
Func: command
Desc: Sends a command to the DOG-Display
Vars: data
------------------------------*/
void dogGraphicDisplay::command(byte dat) 
{
	digitalWrite(p_a0, LOW);
	spi_put_byte(dat);
}

/*----------------------------
Func: data
Desc: Sends data to the DOG-Display
Vars: data
------------------------------*/
void dogGraphicDisplay::data(byte dat) 
{
	 digitalWrite(p_a0, HIGH);
	 spi_put_byte(dat);
}

/*----------------------------
Func: spi_initialize
Desc: Initializes SPI Hardware/Software
Vars: CS-Pin, MOSI-Pin, SCK-Pin (MOSI=SCK Hardware else Software)
------------------------------*/
void dogGraphicDisplay::spi_initialize(byte cs, byte si, byte clk) 
{
	//Set pin Configuration
	p_cs = cs;
	
	if(si == clk)
	{
		hardware = true;
		p_si = MOSI;
		p_clk = SCK;
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
		SPI.setBitOrder(MSBFIRST);
		SPI.setDataMode(SPI_MODE3);
		SPI.setClockDivider(SPI_CLOCK_DIV4);
	}
}

/*----------------------------
Func: spi_put_byte
Desc: Sends one Byte using CS
Vars: data
------------------------------*/
void dogGraphicDisplay::spi_put_byte(byte dat) 
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
void dogGraphicDisplay::spi_put(byte *dat, int len) 
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
void dogGraphicDisplay::spi_out(byte dat) 
{
	byte i = 8;
	if(hardware) 
	{
		SPI.transfer(dat);
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

