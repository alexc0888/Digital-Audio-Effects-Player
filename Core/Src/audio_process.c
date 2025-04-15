#include "audio_process.h"

/* Buffers to store FFT results - currently gets memcpy'ed in fft_driver.c*/
float audioFFTMagBuffer[FFT_LEN / 2];   /* Magnitude of each frequency bin */
float audioFFTFreqBuffer[FFT_LEN / 2];  /* Frequency value of each bin (Hz) */
float audioOutputBuffer[FFT_LEN / 2];
// perform one-time computations outside of functions to reduce computational stress
float bassFreqCutoff = 250.0f; // Hz, adjust to taste
float bassBoostFactor = 2.0f;  // Increase amplitude by this factor, adjust to taste
int cutoffBin = (int)(bassFreqCutoff * FFT_LEN / AUDIO_SAMPLE_RATE);
bool normalizeAudio = FALSE;

// Map the PCM_s16 samples into floating point numbers, on a scale of 0 to 3.3V
// Easier to process and perform FFT on floating point format
void convS16Float(int16_t songBuffer[SONG_BUFF_SIZE], float audioFloat[SONG_BUFF_SIZE], int toFloat)
{
	if(toFloat == TRUE)
	{
		for(int sample = 0; sample < SONG_BUFF_SIZE; sample++)
		{
			audioFloat[sample] = ((float) songBuffer[sample] / (float) MAX_VAL_INT16_T) * MAX_AMPLITUDE; // take as a fraction of 3.3V
		}
	}
	else // float -> int16_t
	{
		for(int sample = 0; sample < SONG_BUFF_SIZE; sample++)
		{
			songBuffer[sample] = (audioFloat[sample] / MAX_AMPLITUDE) * (float) MAX_VAL_INT16_T; // take as a fraction of 3.3V
		}
	}
}

void bassBoost(float audioFloat[SONG_BUFF_SIZE])
{
	// Apply a smooth enhancement curve
	for (int i = 0; i < cutoffBin; i++) {
	    // Apply more boost to lower frequencies with a gradual taper
	    float boostAmount = bassBoostFactor * (1.0f - (float)i / cutoffBin);
	    magnitudes[i] *= (1.0f + boostAmount);
	}

	// Convert back to complex form for Inverse FFT
	for (i = 0; i < FFT_LEN/2; i++) {
	    float real = magnitudes[i] * cos(phases[i]);
	    float imag = magnitudes[i] * sin(phases[i]);
	    audioOutputBuffer[2*i] = real;
	    audioOutputBuffer[2*(i+1)] = imag;
	}

	// Apply inverse FFT to get back to time domain
	arm_rfft_fast_f32(&fftHandler, audioOutputBuffer, audioFloat, 1); // 1 for inverse FFT

	// Normalize if needed to prevent clipping
	if (normalizeAudio == TRUE) {
		float maxAmp = 0.0f;
		for (i = 0; i < SONG_BUFF_SIZE; i++) {
			if (fabs(audioFloat[i]) > maxAmp) maxAmp = fabs(audioFloat[i]);
		}
		if (maxAmp > 1.0f) {
			for (i = 0; i < SONG_BUFF_SIZE; i++) {
				audioFloat[i] /= maxAmp;
			}
		}
	}
}
