#include "audio_process.h"
#include "fft_driver.h" // Add this to access fftHandler

/* Buffers to store FFT results - these are filled via memcpy elsewhere */
float audioFFTMagBuffer[FFT_LEN / 2]; /* Magnitude of each frequency bin */
float audioFFTFreqBuffer[FFT_LEN / 2]; /* Frequency value of each bin (Hz) */
float fftComplexOutput[FFT_LEN]; // Buffer for complex FFT output before inverse transform
// perform one-time computations outside of functions to reduce computational stress
float bassFreqCutoff = 250.0f; // Hz, adjust to taste
float bassBoostFactor = 2.0f; // Increase amplitude by this factor, adjust to taste
int cutoffBin; // Will initialize in init function
uint8_t normalizeAudio = FALSE;

// Add an external reference to fftHandler from fft_driver.c
extern arm_rfft_fast_instance_f32 fftHandler;

/**
 * @brief Initialize audio processing
 * Initialize variables that can't be statically initialized
 */
void initAudioProcess(void)
{
    cutoffBin = (int)(bassFreqCutoff * FFT_LEN / AUDIO_SAMPLE_RATE);
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
 * @brief Apply bass boost to audio signal
 *
 * @param audioFloat Pointer to the audio signal (will be modified in-place)
 *
 * This function assumes the FFT has already been calculated and the results
 * are available in audioFFTMagBuffer and audioFFTFreqBuffer.
 */
void bassBoost(float* audioFloat)
{
    // Apply a smooth enhancement curve to the existing FFT magnitudes
    for(int i = 0; i < cutoffBin; i++) {
        // Apply more boost to lower frequencies with a gradual taper
        float boostAmount = bassBoostFactor * (1.0f - (float)i / cutoffBin);
        audioFFTMagBuffer[i] *= (1.0f + boostAmount);
    }

    // Convert back to complex form for Inverse FFT
    for(int i = 0; i < FFT_LEN/2; i++) {
        // Reconstruct complex FFT data using magnitude and phase
        fftComplexOutput[2*i] = audioFFTMagBuffer[i] * cos(audioFFTFreqBuffer[i]);
        fftComplexOutput[2*i+1] = audioFFTMagBuffer[i] * sin(audioFFTFreqBuffer[i]);
    }

    // Apply inverse FFT to get back to time domain
    arm_rfft_fast_f32(&fftHandler, fftComplexOutput, audioFloat, 1); // 1 for inverse FFT

    // Scale by FFT length (required after inverse FFT)
    for(int i = 0; i < SONG_BUFF_SIZE; i++) {
        audioFloat[i] /= FFT_LEN;
    }

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
