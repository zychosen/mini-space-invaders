/*
 * ssd1306.c
 *
 *  Created on: Aug 22, 2021
 *      Author: zychosen
 */

#include "i2c.h"
#include "ssd1306.h"
#include <string.h>
#include "font5x8.h"

static uint8_t buffer[SIZE];           /* Display buffer */

void ssd1306_setup(void) {
	int i;
	for(i = 100; i > 0; i--);          /* Some delay before sending I2C transaction */
	send_command(DISPLAYOFF);		   /* Turn off display */
	send_command(SETDISPLAYCLOCKDIV);  /* Set clock divide ratio */
	send_command(0x80);                // first 4 bits = 0 --> ratio = 1 and next 4 bits = 8 (default)
	send_command(SETMULTIPLEX);        /* Set multiplex ratio */
	send_command(0x3F);                /* Set to vertical size - 1 */
	send_command(SETDISPLAYOFFSET);    /* Set display offset */
	send_command(0x00);                /* No offset */
	send_command(SETSTARTLINE | 0x0);  /* Start at 0*/
	send_command(CHARGEPUMP);          /* Set charge pump state */
	send_command(0x14);                /* Enable charge pump */
	send_command(MEMORYMODE);          /* Set memory addressing mode */
	send_command(0x00);                /* Horizontal addressing */
	send_command(SEGREMAP | 0x01);     /* Set segment remap */
	send_command(COMSCANDEC);          /* Set COM scan direction to decrement */
	send_command(SETCOMPINS);          /* Set COM pins*/
	send_command(0x12);                /* 0x12 for 128x64 and 0x02 for 128x32 */
	send_command(SETCONTRAST);         /* Set display contrast */
	send_command(0xCF);                /* Contrast value */
	send_command(SETPRECHARGE);        /* Set pre charge period */
	send_command(0xF1);                /* */
	send_command(SETVCOMDETECT);       /* Set COM regulator */
	send_command(0x40);                /* 0.77*Vcc (reset value) */
	send_command(DISPLAYALLON_RESUME); /* Set entire display ON based on GDDRAM contents */
	send_command(NORMALDISPLAY);       /* Set as normal display (not inverted) */
	send_command(DEACTIVATE_SCROLL);   /* Deactivate scroll */
	send_command(DISPLAYON);           /* Turn on OLED display */
	clear_screen();
}

void update_screen(void) {
	send_command(PAGEADDR);                       /* Set page start and end address */
	send_command(0x0);                            /* Start at page 0 */
	send_command(0xFF);                           /* Page end address */
	send_command(COLUMNADDR);                     /* Set column start and end address */
	send_command(0);                              /* Column starts at 0 */
	send_command(WIDTH - 1);                      /* Column end at 127 */

	i2c_start();
	i2c_transmit_byte(0x40);                      /* Indicates that data bytes will be transmitted */
	i2c_transmit_multiple_bytes(buffer, SIZE);
	i2c_stop();
}

void clear_screen(void) {
	memset(buffer, 0, SIZE);
}

void draw_pixel(uint8_t x, uint8_t y, uint8_t colour) {
	if (y >= HEIGHT || x >= WIDTH) return;
	if (colour == WHITE) {
		buffer[x + (uint16_t)((y >> 3) << 7)] |= (1 << (y & 7));      /* Mapping of coordinates --> buffer index; y >> 3 as each page consists of 8 bits */
	} else {
		buffer[x + (uint16_t)((y >> 3) << 7)] &= ~(1 << (y & 7));
	}
}

void send_command(uint8_t cmd) {
	int i;

	i2c_start();					    //                 MSB                            7                                    6-0     ----------------
	i2c_transmit_byte((uint8_t)0x00);   // command format: C0(0-only data bytes further)  0-next byte is command else is data  0            <next byte>
	i2c_transmit_byte(cmd);
	i2c_stop();

	/* delay for a bit */
	for(i = 10; i > 0; i--);
}

void drawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
	int8_t dx = x2 - x1;
	int8_t dy = y2 - y1;

	/* Draw horizontal line */
	if(!dy) {
		while (x1 <= x2) {
			draw_pixel(x1, y1, 1);
			x1++;
		}

	/* Draw vertical line */
	} else if(!dx) {
		while(y1 <= y2) {
			draw_pixel(x1, y1, WHITE);
			y1++;
		}

	} else {
		/* Naive algorithm
		int x,y;
		for(x = x1; x <= x2; x++) {
			y = y1 + dy*(x-x1)/dx;
			draw_pixel(x, y, 1);
		} */

		/* Bresenham's algorithm */
		int D = 2*dy - dx;
		int x, y = y1;
		for(x = x1; x <= x2; x++) {
			draw_pixel(x, y, WHITE);
			if(D > 0) {
				y++;
				D -= 2*dx;
			}
			D += 2*dy;
		}
	}
}

void drawChar(char c, uint8_t x, uint8_t y) {
	uint8_t _char = c - 32;            /* Offset of 32 relative to ASCII values */
	uint8_t _y = y;
	uint8_t i,k;
	for(k = 0; k < FONT_WIDTH; k++) {
		y = _y;
		for(i = 0; i < FONT_HEIGHT; i++) {
			if((ASCII[_char][k] >> i) & 0x01) {
				draw_pixel(x, y, WHITE);
			}
			y++;
		}
		x++;
	}
}

void drawString(char *s, uint8_t x, uint8_t y) {
	uint8_t j = 0;
	while (s[j] != '\0') {
		drawChar(s[j], x, y);
		j++; x += FONT_WIDTH + 1;           /* Increment x position after each character is printed to avoid overlap of characters */
	}
}

void drawBitmap(uint8_t const *bitmap, uint8_t w, uint8_t h, uint8_t x, uint8_t y) {
	int j, _x = x, byteNum = 0, x_max = w + x, y_max = y + h;
	for(; y < y_max; y++) {
		x = _x;
		while (x < x_max) {
			for(j = 0; j < 8; j++, x++) {
				if((bitmap[byteNum] >> (8 - j - 1)) & 0x01)
				draw_pixel(x,y,1);
			}
			++byteNum;
		}
	}
}

