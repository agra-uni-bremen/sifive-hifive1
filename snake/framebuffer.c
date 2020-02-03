/*
 * framebuffer.c
 *
 *  Created on: Feb 3, 2020
 *      Author: dwd
 */
#include "framebuffer.h"


uint8_t framebuffer[DISP_W][DISP_H/8];

void fb_init()
{
	memset(framebuffer, 0, DISP_W*(DISP_H/8));
}
void fb_flush()
{
	set_xy(0,0);
	for(uint8_t x = 0; x < DISP_W; x++)
	{
		for(uint8_t y = 0; y < DISP_H/8; y++)
		{
			spi(framebuffer[x][y]);
		}
	}
}

void fb_set_pixel(uint8_t x, uint8_t y, uint8_t p)
{
	if(p)
		framebuffer[x][y/8] |= 1 << y%8;
	else
		framebuffer[x][y/8] &= ~(1 << y%8);
}

uint8_t fb_get_pixel(uint8_t x, uint8_t y)
{
	return framebuffer[x][y/8] & (1 << y%8);
}
