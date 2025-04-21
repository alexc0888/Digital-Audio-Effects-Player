#include "fft_driver.h"

/* Global FFT handler from ARM CMSIS DSP library */
arm_rfft_fast_instance_f32 fftHandler;

/**
 * @brief Initialize the FFT module
 *
 * This function initializes the ARM CMSIS DSP FFT handler with the specified
 * FFT length (defined in FFT_LEN).
 */
void initFFT(uint16_t fftLen)
{
    arm_rfft_fast_init_f32(&fftHandler, fftLen);
}

/**
 * @brief Perform FFT calculation on input signal
 *
 * @param inputSignal Pointer to the input signal data
 * @param inputSize Size of the input signal array
 * @param fftMagBuffer Buffer to store magnitude results (output)
 * @param fftFreqBuffer Buffer to store frequency values (output)
 *
 * This function performs FFT on the input signal and calculates
 * the magnitude and corresponding frequency for each FFT bin.
 */
void computeFFT(float_t * inputSignal, uint16_t inputSize, uint16_t fftLength, float * fftMagBuffer, float * fftFreqBuffer, float * fftPhaseBuffer)
{
		if((fftLength > FFT_MAX_LEN) || ((fftLength & (fftLength - 1)) != 0))
		{
			DEBUG_PRINTF("ERROR (computeFFT): FFT Length must be a power of 2 and may not exceed a length of %d", FFT_MAX_LEN);
			return;
		}

    float fftInputBuffer[FFT_MAX_LEN];   /* Buffer for FFT input */
    float fftOutputBuffer[FFT_MAX_LEN];  /* Buffer for FFT output (complex values) */
		initFFT(fftLength); // setup the FFT length

    /* Copy input signal to FFT input buffer with zero padding */
    for(int index = 0; index < fftLength; index++)
    {
        if(index < inputSize)
        {
            fftInputBuffer[index] = inputSignal[index];
        }
        else
        {
            /* Zero-pad if input signal is shorter than FFT length */
            fftInputBuffer[index] = 0;
        }
    }

    /* Perform real FFT using ARM CMSIS DSP library */
    arm_rfft_fast_f32(&fftHandler, fftInputBuffer, fftOutputBuffer, 0);

    /* Process FFT results (only half of bins due to Nyquist limit) */
    for(int index = 0; index < fftLength / 2; index++)
    {
        /* Calculate magnitude using Pythagorean theorem (sqrt(real² + imag²)) */
        /* Real part is at even indices, imaginary part at odd indices */
        fftMagBuffer[index] = sqrt(
            (fftOutputBuffer[2*index] * fftOutputBuffer[2*index]) +
            (fftOutputBuffer[2*index + 1] * fftOutputBuffer[2*index + 1])
        ) / (fftLength / 2); /* Normalize magnitude */

        /* Calculate the actual frequency in Hz for this bin */
        fftFreqBuffer[index] = (float)((index * AUDIO_SAMPLE_RATE)) / ((float)fftLength);

        /* Calculate the phase information of the singal in Radians */
        fftPhaseBuffer[index] = (float) atan2(fftOutputBuffer[2*index + 1], fftOutputBuffer[2*index]);
    }
}



void computeIFFT(float_t * timeDomainSignal, uint16_t timeDomainSize, uint16_t fftLength, float * fftMagBuffer, float * fftPhaseBuffer)
{
	if((fftLength > FFT_MAX_LEN) || ((fftLength & (fftLength - 1)) != 0))
	{
			DEBUG_PRINTF("ERROR (computeIFFT): FFT Length must be a power of 2 and may not exceed a length of %d", FFT_MAX_LEN);
			return;
	}
	// clear out the old time-domain signal
//	for(int sample = 0; sample < timeDomainSize; sample++)
//	{
//		timeDomainSignal[sample] = 0;
//	}
	float fftComplexOutput[FFT_MAX_LEN]; // Buffer for complex FFT output before inverse transform
	initFFT(fftLength);

	// Convert back to complex form for Inverse FFT
	for(int i = 0; i < FFT_MAX_LEN; i++)
	{
			if(i < fftLength / 2)
			{
				// Reconstruct complex FFT data using magnitude and phase
				fftComplexOutput[2*i]   = fftMagBuffer[i] * cos(fftPhaseBuffer[i]);
				fftComplexOutput[2*i+1] = fftMagBuffer[i] * sin(fftPhaseBuffer[i]);
			}
			else // taking a smaller pt IFFT than the maximum allows, so fill the rest of the array with zeros
			{
				fftComplexOutput[i] = 0;
			}

	}

	// Apply inverse FFT to get back to time domain
	arm_rfft_fast_f32(&fftHandler, fftComplexOutput, timeDomainSignal, 1); // 1 for inverse FFT

	// Scale by FFT length (required after inverse FFT)
	// Disabled because we already normalize the magnitude after performing the FFT in fft_driver.c
	// for(int i = 0; i < SONG_BUFF_SIZE; i++)
	// {
	   // audioFloat[i] /= FFT_AUDIO_LEN;
	// }
}






