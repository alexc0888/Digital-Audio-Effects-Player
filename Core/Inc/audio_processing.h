#ifndef __AUDIO_PROCESSING__
#define __AUDIO_PROCESSING__

#include "shared_consts.h"
#include "stm32f4xx_hal.h"

// defined constants


// function declarations
void convS16Float(int16_t[SONG_BUFF_SIZE], float[SONG_BUFF_SIZE], int toFloat);


#endif
