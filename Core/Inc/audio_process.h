#ifndef __AUDIO_PROCESS__
#define __AUDIO_PROCESS__

#define ARM_MATH_CM4  // Add this BEFORE including arm_math.h
#include "arm_math.h"
#include "shared_consts.h"
#include "stm32f4xx_hal.h"
#include <math.h>
#include "stm32f413xx.h"
#include "fft_driver.h"

// External variables
extern float audioFFTMagBuffer[FFT_LEN / 2];   /* Magnitude of each frequency bin */
extern float audioFFTFreqBuffer[FFT_LEN / 2];  /* Frequency value of each bin (Hz) */
extern float audioOutputBuffer[FFT_LEN / 2];
extern uint8_t normalizeAudio;

// defined constants


// function declarations
void convS16Float(int16_t* songBuffer, float* audioFloat, int toFloat);
void bassBoost(float* audioFloat);

#endif
