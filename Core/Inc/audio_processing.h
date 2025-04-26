/*
 * audio_processing.h
 *
 *  Created on: Apr 12, 2025
 *      Authors: derekxrosales (Derek Rosales)
 *       				 alexc0888 (Alex Chitsazzadeh)
 */
#ifndef __AUDIO_PROCESSING__
#define __AUDIO_PROCESSING__

#include "shared_consts.h"
#include "stm32f4xx_hal.h"
#include <math.h>
#include "stm32f413xx.h"
#include "fft_driver.h"


// External variables
extern float audioFFTMagBuffer[FFT_AUDIO_LEN / 2];   /* Magnitude of each frequency bin */
extern float audioFFTFreqBuffer[FFT_AUDIO_LEN / 2];  /* Frequency value of each bin (Hz) */
extern float audioFFTPhaseBuffer[FFT_AUDIO_LEN / 2]; /* Phase value of each bin (Rad) */
extern uint8_t normalizeAudio;
extern uint8_t FDbassBoostEnabled;
extern uint8_t TDbassBoostEnabled;

// defined constants
#define BASS_BOOST_FACTOR_MAX 10.0f
#define BASS_BOOST_CUTOFF_MAX 0.1f
#define TREMOLO_DEPTH_MAX     1.0f


// function declarations
void convS16Float(int16_t* songBuffer, float* audioFloat, int toFloat);
void applyAudioEffects(float* audioFloat, uint16_t inputSize, knobs_t audioParams);
void FDbassBoost();
void TDbassBoost();
void initAudioProcess(knobs_t audioParams);
void TDsetBassBoostParameters(float amount, float cutoff);
void TDtoggleBassBoost(void);
void setVolume(float* audioFloat, float gain);
void tremoloFilter(float* audioFloat);

#endif
