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

#ifndef DOGGRAPHICDISPLAY_H
#define DOGGRAPHICDISPLAY_H


#define DOGM128 1
#define DOGL128 2
#define DOGM132 3
#define DOGS102 4

#define ALIGN_LEFT 1
#define ALIGN_RIGHT 2
#define ALIGN_CENTER 3

#define STYLE_NORMAL 1
#define STYLE_FULL 2
#define STYLE_INVERSE 3
#define STYLE_FULL_INVERSE 4

#define VIEW_BOTTOM 0xC0
#define VIEW_TOP 	0xC8


class dogGraphicDisplay
{
  public:
    void initialize     (byte p_cs, byte p_si, byte p_clk, byte p_a0, byte p_res, byte type);
    void clear			(void);
    void contrast       (byte contr);
	void view		(byte direction);
	void all_pixel_on	(bool state);
	void inverse		(bool state);
	void sleep		(bool state);
	void string (byte column, byte page, const byte *font_adress, const char *str);
	void string (byte column, byte page, const byte *font_adress, const char *str, byte align);
	void string (byte column, byte page, const byte *font_adress, const char *str, byte align, byte style);
	void stringx         (byte column, byte page, int offset, const byte *font_adress, const char *str, byte align, byte style);
	void rectangle		(byte start_column, byte start_page, byte end_column, byte end_page, byte pattern);
	void picture		(byte column, byte page, const byte *pic_adress);
	byte display_width (void);

  private:
    byte p_cs;
    byte p_si;
    byte p_clk;
    byte p_a0;
	byte type;
	boolean hardware;
    boolean top_view;
	
	void position   (byte column, byte page);
    void command	(byte dat);
    void data		(byte dat);
    
    void spi_initialize	(byte cs, byte si, byte clk);
    void spi_put_byte	(byte dat);
    void spi_put		(byte *dat, int len);
	void spi_out		(byte dat);
};

#endif /* DOGGRAPHICDISPLAY_H */
