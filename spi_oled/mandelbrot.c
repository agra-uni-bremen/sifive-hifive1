/*
 * mandelbrot.c
 *
 *  Created on: 18 Sep 2019
 *      Author: dwd
 */
#include "mandelbrot.h"
#include "display.h"

/* Is a point on the mandelbrot set interesting to zoom in on? */
int interesting(fp_t x, fp_t y)
{
    return 1; /* TODO */
}

void mandelbrot(uint8_t (*wait_condition)(void))
{
    char c;
    int frame = 0;
    fp_t centerx = I(-1)/2;
    fp_t centery = I(3)/4;
    fp_t start_radiusx = I(3);
    fp_t start_radiusy = I(1);
    fp_t radiusx = start_radiusx;
    fp_t radiusy = start_radiusy;
    setContrast(0);
    while (!(wait_condition)()) {
		if (frame == 0) {
			do {
				centerx = ((fp_t)mrand48()<<(PREC-31)) + I(-1)/2;
				centery = ((fp_t)mrand48()<<(PREC-32));
			} while (!interesting(centerx, centery));
			radiusx = start_radiusx;
			radiusy = start_radiusy;
		}
		fp_t basex = centerx - radiusx;
		fp_t basey = centery - radiusy;
		fp_t stepx = 2 * radiusx / DISP_W;
		fp_t stepy = 2 * radiusy / DISP_H;

		if (radiusx < (I(1)>>4) || radiusy < (I(1)>>4)) {
			frame = 0;
			continue;
		}
		uint8_t none = 0xff;
		uint8_t any = 0x00;
		for (int row=0; row<DISP_H/8; ++row) {
			set_xy(0, row);
			for (int x=0; x<DISP_W; ++x) {
				uint8_t byte = 0;
				for (int yi=0; yi<8; ++yi) {
					int y = row*8+yi;
					fp_t cx = basex + x * stepx;
					fp_t cy = basey + y * stepy;
					/* Z = 0 */
					fp_t zx = I(0);
					fp_t zy = I(0);
					int it;
					for (it=0; it<ITMAX; ++it) {
						fp_t zx2 = MUL(zx,zx);
						fp_t zy2 = MUL(zy,zy);
						/* |Z| <= 2 */
						if (zx2 + zy2 > I(4)) {
							break;
						}
						/* Z = Z^2 + C */
						fp_t twozxy = 2 * MUL(zx,zy);
						zx = zx2 - zy2 + cx;
						zy = twozxy + cy;
					}

					//int bit = it < itmax;
					int bit = it&1;
					byte |= (bit << yi);
				}
				spi(byte);
				any |= byte;
				none &= byte;
			}
		}
		if(frame == 0)
			fadeIn(500);
		if (any == 0x00 || none == 0xff) {
			/* If screen empty or full, restart */
			if(none == 0xff)
				fadeOut(500);
			frame = 0;
			continue;
		}
		frame += 1;
		radiusx = (radiusx * (ZOOM_MUL-1))/ZOOM_MUL;
		radiusy = (radiusy * (ZOOM_MUL-1))/ZOOM_MUL;
	}
}
