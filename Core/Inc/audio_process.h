#ifndef __AUDIO_PROCESS__
#define __AUDIO_PROCESS__

#include "shared_consts.h"
#include "stm32f4xx_hal.h"


// External variables
extern float audioFFTMagBuffer[FFT_LEN / 2];   /* Magnitude of each frequency bin */
extern float audioFFTFreqBuffer[FFT_LEN / 2];  /* Frequency value of each bin (Hz) */
extern float audioOutputBuffer[FFT_LEN / 2];
extern bool normalizeAudio;

// defined constants


// function declarations
void convS16Float(int16_t[SONG_BUFF_SIZE], float[SONG_BUFF_SIZE], int toFloat);


#endif
