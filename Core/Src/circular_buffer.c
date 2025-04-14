#include "circular_buffer.h"



void initCircBuff(int buffSize, circ_buff_t *buffStatus)
{
	buffStatus -> empty = TRUE;
	buffStatus -> full  = FALSE;
	buffStatus -> rdPtr = 0;
	buffStatus -> wrPtr = 0;
	buffStatus -> size  = buffSize;
}

void pushCircBuff(circ_buff_t *buffStatus)
{
	buffStatus -> wrPtr++;
	if(buffStatus -> wrPtr == buffStatus -> size) // wrap-around to start of circular buffer
	{
		buffStatus -> wrPtr = 0;
	}

	// update the full field if needed since we just pushed
	buffStatus -> full = ((buffStatus -> wrPtr) == (buffStatus -> rdPtr - 1)) ||
										   ((buffStatus -> wrPtr == (buffStatus  -> size - 1)) && (buffStatus -> rdPtr == 0)) ? TRUE : FALSE;
	buffStatus -> empty = FALSE;
}

void popCircBuff(circ_buff_t *buffStatus)
{
	buffStatus -> rdPtr++;
	if(buffStatus -> rdPtr == buffStatus -> size)
	{
		buffStatus -> rdPtr = 0;
	}

	// update the empty field if needed since we just popped
	buffStatus -> empty = (buffStatus -> rdPtr != buffStatus -> wrPtr) ? TRUE : FALSE;
	buffStatus -> full  = FALSE;
}
