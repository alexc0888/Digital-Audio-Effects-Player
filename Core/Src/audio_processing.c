#include "audio_processing.h"

/* Buffers to store FFT results - these are filled via memcpy elsewhere */
float audioFFTMagBuffer[FFT_AUDIO_LEN / 2]; /* Magnitude of each frequency bin */
float audioFFTFreqBuffer[FFT_AUDIO_LEN / 2]; /* Frequency value of each bin (Hz) */
float audioFFTPhaseBuffer[FFT_AUDIO_LEN / 2]; /* Phase value of each bin (Rad) */

// perform one-time computations outside of functions to reduce computational stress
float bassFreqCutoff = 250.0f; // Hz, adjust to taste
float bassBoostFactor = 8.0f; // Increase amplitude by this factor, adjust to taste
int cutoffBin; // Will initialize in init function
uint8_t normalizeAudio = FALSE;

/**
 * @brief Initialize audio processing
 * Initialize variables that can't be statically initialized
 */
void initAudioProcess(void)
{
    cutoffBin = (int)(bassFreqCutoff * FFT_AUDIO_LEN / AUDIO_SAMPLE_RATE);

}

// Map the PCM_s16 samples into floating point numbers, on a scale of 0 to 3.3V
// Easier to process and perform FFT on floating point format
void convS16Float(int16_t* songBuffer, float* audioFloat, int toFloat)
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
            songBuffer[sample] = (int16_t)((audioFloat[sample] / MAX_AMPLITUDE) * (float) MAX_VAL_INT16_T); // take as a fraction of 3.3V
        }
    }
}
/**
 * This function will apply various audio effects such as bass and treble boosting.
 * This will be performed via an FFT. Before returning to main, an IFFT is performed
 * to get the processed time-domain signal back.
 */
void applyAudioEffects(float* audioFloat, uint16_t inputSize)
{
	computeFFT(audioFloat, inputSize, FFT_AUDIO_LEN, audioFFTMagBuffer, audioFFTFreqBuffer, audioFFTPhaseBuffer);
	bassBoost();
  computeIFFT(audioFloat, inputSize, FFT_AUDIO_LEN, audioFFTMagBuffer, audioFFTPhaseBuffer);

  // Normalize if needed to prevent clipping
  if(normalizeAudio == TRUE) {
      float maxAmp = 0.0f;
      for(int i = 0; i < SONG_BUFF_SIZE; i++) {
          if(fabs(audioFloat[i]) > maxAmp) maxAmp = fabs(audioFloat[i]);
      }
      if(maxAmp > 1.0f) {
          for(int i = 0; i < SONG_BUFF_SIZE; i++) {
              audioFloat[i] /= maxAmp;
          }
      }
  }
}

/**
 * @brief Apply bass boost to audio signal
 *
 * This function assumes the FFT has already been calculated and the results
 * are available in audioFFTMagBuffer
 */
void bassBoost()
{
    // Apply a smooth enhancement curve to the existing FFT magnitudes
    for(int i = 0; i <= cutoffBin; i++)
    {
        // Apply more boost to lower frequencies with a gradual taper
        float boostAmount = bassBoostFactor * (1.0f - (float)i / (cutoffBin + 1));
        audioFFTMagBuffer[i] *= (1.0f + boostAmount);
    }
}
