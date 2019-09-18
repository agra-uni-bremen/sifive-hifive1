/*
 * mandelbrot.h
 *
 *  Created on: 18 Sep 2019
 *      Author: dwd
 */

#pragma once

#include <stdint.h>

/* Mandelbrot */
#define PREC 48  /* number of precision bits */
#define ITMAX 16 /* number of iterations */
#define I(x) (((int64_t)(x))<<PREC) /* integral value */
#define FRAC(x,y) (I(x)/(y)) /* fraction */
/* stupid multiplication of two fp_t */
#define MUL(x,y)  (((x)>>(PREC/2)) * ((y)>>(PREC/2)))
#define ZOOM_MUL (256L)
typedef int64_t fp_t;


void mandelbrot();

