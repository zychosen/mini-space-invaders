/*
 * ssd1306.h
 *
 *  Created on: Aug 22, 2021
 *      Author: zychosen
 */

#ifndef SSD1306_H_
#define SSD1306_H_

#include <stdint.h>

#define BLACK 0
#define WHITE 1

#define WIDTH 128
#define HEIGHT 64
#define SIZE (WIDTH*HEIGHT)/8

#define MEMORYMODE 0x20
#define COLUMNADDR 0x21
#define PAGEADDR 0x22
#define SETCONTRAST 0x81
#define CHARGEPUMP 0x8D
#define SEGREMAP 0xA0
#define DISPLAYALLON_RESUME 0xA4
#define NORMALDISPLAY 0xA6
#define INVERTDISPLAY 0xA7
#define SETMULTIPLEX 0xA8
#define DISPLAYOFF 0xAE
#define DISPLAYON 0xAF
#define COMSCANDEC 0xC8
#define SETDISPLAYOFFSET 0xD3
#define SETDISPLAYCLOCKDIV 0xD5
#define SETPRECHARGE 0xD9
#define SETCOMPINS 0xDA
#define SETVCOMDETECT 0xDB
#define SETSTARTLINE 0x40

#define EXTERNALVCC 0x01
#define SWITCHCAPVCC 0x02

#define RIGHT_HORIZONTAL_SCROLL 0x26
#define LEFT_HORIZONTAL_SCROLL 0x27
#define VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29
#define VERTICAL_AND_LEFT_HORIZONTAL_SCROLL 0x2A
#define DEACTIVATE_SCROLL 0x2E
#define ACTIVATE_SCROLL 0x2F
#define SET_VERTICAL_SCROLL_AREA 0xA3
#define SET_LCOL_START_ADDRESS 0x00
#define SET_HCOL_START_ADDRESS 0x10
#define SET_PAGE_START_ADDRESS 0xB0

void ssd1306_setup(void);
void update_screen(void);
void clear_screen(void);
void send_command(uint8_t);
void draw_pixel(uint8_t, uint8_t, uint8_t);
void drawLine(uint8_t, uint8_t, uint8_t, uint8_t);
void drawCircle(uint8_t, uint8_t, uint8_t);
void drawBitmap(uint8_t const *, uint8_t, uint8_t, uint8_t, uint8_t);

#endif /* SSD1306_H_ */
