/*
 * morse.h
 *
 *  Created on: 18 Jan 2019
 *      Author: dwd
 */

#ifndef MORSE_H
#define MORSE_H

#include <stdint.h>

enum MorsePulse
{
	none = 0,
	shortt,
	longg,
};

#define MORSE_MAXLEN 5

char findChar(enum MorsePulse list[MORSE_MAXLEN + 1]);

#endif


