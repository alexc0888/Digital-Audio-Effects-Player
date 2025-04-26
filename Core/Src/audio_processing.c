#include "audio_processing.h"

/* Buffers to store FFT results - these are filled via memcpy elsewhere */
float audioFFTMagBuffer[FFT_AUDIO_LEN / 2]; /* Magnitude of each frequency bin */
float audioFFTFreqBuffer[FFT_AUDIO_LEN / 2]; /* Frequency value of each bin (Hz) */
float audioFFTPhaseBuffer[FFT_AUDIO_LEN / 2]; /* Phase value of each bin (Rad) */

// Frequency Domain Approach params and variables
float FDbassFreqCutoff = 250.0f; // Hz, adjust to taste
float FDbassBoostFactor = 8.0f; // Increase amplitude by this factor, adjust to taste
int cutoffBin; // Will initialize in init function
uint8_t FDbassBoostEnabled = FALSE;

// Filter state variables for time domain approach
float TDfilterState = 0.0f;  // For low-pass filter - save the previous value
uint8_t TDbassBoostEnabled = TRUE;
float TDbassBoostFactor = 3.0f;  // Amplification factor for bass frequencies
float TDfilterCoeff = 0.1f;     // Lower value = lower cutoff frequency (more bass)


// Filter state variables for tremolo effect
uint8_t tremoloFilterEnabled = TRUE;
float triangleWave = 0.0f;
float triDir       = 1.0f;
float triFreq      = 10;
float triMax       = 0;
float tremoloDepth = 0.5f;


uint8_t normalizeAudio = TRUE;


/**
 * @brief Initialize audio processing
 * Initialize variables that can't be statically initialized
 */
void initAudioProcess(knobs_t audioParams)
{
    if (FDbassBoostEnabled)
    {
  		cutoffBin = (int)(FDbassFreqCutoff * FFT_AUDIO_LEN / AUDIO_SAMPLE_RATE);
    }
    // Reset filter state
    else if (TDbassBoostEnabled)
    {
  		TDfilterState = 0.0f;
  		TDbassBoostFactor = audioParams.bassGain * BASS_BOOST_FACTOR_MAX;
  		TDfilterCoeff     = audioParams.bassCutoff * BASS_BOOST_CUTOFF_MAX;
    }
    if (tremoloFilterEnabled)
    {
      triMax =  (AUDIO_SAMPLE_RATE / triFreq) * 0.25f;
      tremoloDepth = audioParams.tremoloDepth * TREMOLO_DEPTH_MAX;
    }
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
void applyAudioEffects(float* audioFloat, uint16_t inputSize, knobs_t audioParams)
{
	initAudioProcess(audioParams);

	setVolume(audioFloat, 8.3f);
	if(tremoloFilterEnabled)
	{
		tremoloFilter(audioFloat);
	}
	if (TDbassBoostEnabled)
	{
		TDbassBoost(audioFloat);
		// Normalize if needed to prevent clipping
	    if(normalizeAudio)
	    {
	        float maxAmp = 0.0f;

	        // Find maximum amplitude
	        for(int i = 0; i < SONG_BUFF_SIZE; i++)
	        {
	            float absValue = fabsf(audioFloat[i]);
	            if(absValue > maxAmp) maxAmp = absValue;
	        }

	        // Scale if above threshold
	        if(maxAmp > 0.95f * MAX_AMPLITUDE)
	        {
	            float scale = (0.95f * MAX_AMPLITUDE) / maxAmp;
	            for(int i = 0; i < SONG_BUFF_SIZE; i++)
	            {
	                audioFloat[i] *= scale;
	            }
	        }
	    }
	}
	else if (FDbassBoostEnabled)
	{
		computeFFT(audioFloat, inputSize, FFT_AUDIO_LEN, audioFFTMagBuffer, audioFFTFreqBuffer, audioFFTPhaseBuffer);
		FDbassBoost();
		computeIFFT(audioFloat, inputSize, FFT_AUDIO_LEN, audioFFTMagBuffer, audioFFTPhaseBuffer);
		// Normalize if needed
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

  // Normalize if needed to prevent clipping

}

/**
 * @brief Apply bass boost to audio signal
 *
 * This function assumes the FFT has already been calculated and the results
 * are available in audioFFTMagBuffer
 */
void FDbassBoost()
{
    // Apply a smooth enhancement curve to the existing FFT magnitudes
    for(int i = 0; i <= cutoffBin; i++)
    {
        // Apply more boost to lower frequencies with a gradual taper
        float boostAmount = FDbassBoostFactor * (1.0f - (float)i / (cutoffBin + 1));
        audioFFTMagBuffer[i] *= (1.0f + boostAmount);
    }
}


/**
 * @brief Apply bass boost using a time-domain approach
 * @param audioFloat Audio buffer (in-place processing)
 *
 * This function:
 * 1. Extracts bass frequencies using a low-pass filter
 * 2. Amplifies the bass component
 * 3. Adds the boosted bass back to the original signal
 */
void TDbassBoost(float* audioFloat)
{

    // Create a temporary buffer for the bass component
    float bassComponent[SONG_BUFF_SIZE];

    // Extract bass frequencies using low-pass filter
    bassComponent[0] = TDfilterCoeff * audioFloat[0] + (1.0f - TDfilterCoeff) * TDfilterState;
    TDfilterState = bassComponent[0];

    for(int i = 1; i < SONG_BUFF_SIZE; i++)
    {
        // First-order IIR low-pass filter
        bassComponent[i] = TDfilterCoeff * audioFloat[i] + (1.0f - TDfilterCoeff) * bassComponent[i-1];
    }

    // Add amplified bass back to original signal
    for(int i = 0; i < SONG_BUFF_SIZE; i++)
    {
        audioFloat[i] += TDbassBoostFactor * bassComponent[i];
    }
}

/**
 * @brief Set bass boost parameters - to be used later when user input is dynamic (maybe)
 * @param amount Boost amount (0.0 = no boost, >1.0 = boost)
 * @param cutoff Cutoff coefficient (0.01-0.1, lower = deeper bass)
 */
void TDsetBassBoostParameters(float amount, float cutoff)
{
    TDbassBoostFactor = amount;
    TDfilterCoeff = cutoff;
}

/**
 * @brief Toggle bass boost effect on/off, propbably via button or screen press
 */
void TDtoggleBassBoost(void)
{
    TDbassBoostEnabled = !TDbassBoostEnabled;
}

/**
 * @brief Adjust the overall volume of the outgoing sample
 * @param audioFloat - Audio Buffer
 * @param gain - [0, 1) reduces the volume, (1, inf) increases the volume, [1, 1] maintains it
 */
void setVolume(float* audioFloat, float gain)
{
	// don't waste time if applying unity gain
	if(gain != 1.0f) // 1.0f is exactly representable in IEEE float, so this is okay to do
	{
		for(int sample = 0; sample < SONG_BUFF_SIZE; sample++)
		{
			audioFloat[sample] *= gain;
			// do not allow sample to exceed 95% of MAX_AMPLITUDE, else we will have overflow when converting back to int16_t in dac_output.c
			if(audioFloat[sample] > 0.95f * MAX_AMPLITUDE)
			{
				audioFloat[sample] = 0.95 * MAX_AMPLITUDE;
			}
		}
	}
}

void tremoloFilter(float* audioFloat)
{
	for(int sample = 0; sample < SONG_BUFF_SIZE; sample++)
	{
		audioFloat[sample] = audioFloat[sample] * ((1.0f - tremoloDepth) + (tremoloDepth * triangleWave / triMax)); // bound to [-1, 1]
		triangleWave += triDir;

		if(triangleWave >= triMax || triangleWave <= -1 * triMax)
		{
			triDir *= -1; // reached a peak of the wave period, so flip the sign to go the other way
		}
	}
}


