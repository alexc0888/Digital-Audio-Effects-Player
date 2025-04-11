#ifndef __FFT_DRIVER__
#define __FFT_DRIVER__

#define ARM_MATH_CM4 // define Cortex M4 for arm_math.h

#include <math.h>
#include "stm32f413xx.h"
#include "arm_math.h"
#include "matrix_driver.h"


#define AUDIO_SAMPLE_RATE 44100
#define MAX_AMPLITUDE 3.3 // 1 for now, but this really depends on bit-depth of the wav file and potentiometer

#define FFT_LEN 32
// strange things happen if we don't cast this define to an int
#define BIN_WIDTH_SCREEN (int) (COL / (FFT_LEN / 2)) // the amount of pixel columns taken up per bin on screen


void initFFT(void);
void computeFFT(float *, uint16_t, float *, float *);
void computeFFTScreen(float *, uint16_t, color_t [ROW][COL]);


#endif
