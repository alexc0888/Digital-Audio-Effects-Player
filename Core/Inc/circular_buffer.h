/*
 * circular_buffer.h
 *
 *  Created on: Apr 14, 2025
 *      Author: alexc0888 (Alex Chitsazzadeh)
 */

#ifndef __CIRCULAR_BUFFER__
#define __CIRCULAR_BUFFER__

#include "shared_consts.h"

// Contains important status information for a given circular buffer
typedef struct
{
	int wrPtr;
	int rdPtr;
	int empty;
	int full;
	int size;
} circ_buff_t;

void initCircBuff(int, circ_buff_t*);
void pushCircBuff(circ_buff_t *buffStatus);
void popCircBuff(circ_buff_t *buffStatus);


#endif
