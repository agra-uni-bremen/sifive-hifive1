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

typedef struct BTreeNode
{
	char val;
	struct BTreeNode* shortt;
	struct BTreeNode* longg;
} BTreeNode;

char findChar(enum MorsePulse list[MORSE_MAXLEN + 1]);

#endif


