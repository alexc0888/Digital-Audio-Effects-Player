#include "fft_driver.h"
#include "audio_process.h"
/* Global FFT handler from ARM CMSIS DSP library */
arm_rfft_fast_instance_f32 fftHandler;

/**
 * @brief Initialize the FFT module
 *
 * This function initializes the ARM CMSIS DSP FFT handler with the specified
 * FFT length (defined in FFT_LEN).
 */
void initFFT(void)
{
    arm_rfft_fast_init_f32(&fftHandler, FFT_LEN);
}

/**
 * @brief Compute FFT and prepare visualization data for the LED matrix
 *
 * @param inputSignal Pointer to input signal data
 * @param inputSize Size of the input signal array
 * @param frame 2D array to store color data for the LED matrix
 *
 * This function processes the input signal through FFT, maps the frequency
 * magnitudes to heights on the LED display, and fills the frame buffer with
 * appropriate color values for visualization.
 */
void computeFFTScreen(float * inputSignal, uint16_t inputSize, color_t frame[ROW][COL])
{
    /* Buffers to store FFT results */
    float fftMagBuffer[FFT_LEN / 2];   /* Magnitude of each frequency bin */
    float fftFreqBuffer[FFT_LEN / 2];  /* Frequency value of each bin (Hz) */

    /* Buffer to store scaled magnitudes (heights) for display */
    int binMagBuffer[FFT_LEN / 2];
    uint8_t bin;

    /* Perform FFT calculation */
    computeFFT(inputSignal, inputSize, fftMagBuffer, fftFreqBuffer);

    /* Copy the outputs to extern variables contained in audio_process.c - this is messy but more efficient, needs cleaning tho */
    memcpy(audioFFTMagBuffer,fftMagBuffer,sizeof(float) * (FFT_LEN/2));
    memcpy(audioFFTFreqBuffer,fftFreqBuffer,sizeof(float) * (FFT_LEN/2));

    /* Convert FFT magnitudes to pixel heights (scaled to fit LED matrix) */
    for(bin = 0; bin < FFT_LEN / 2; bin++)
    {
        /* Scale magnitude to height in pixels (0 to ROW) */
        binMagBuffer[bin] = round(fftMagBuffer[bin] * (float) ROW);
    }
    bin = 0; /* Reset bin counter for the display mapping loop */

    /* Map FFT data to the LED matrix, starting from bottom row up */
    for(int row = ROW - 1; row >= 0; row--)
    {
        for(int col = 0; col < COL; col++)
        {
            /* If current bin has magnitude at this height, color the pixel */
            if(binMagBuffer[bin] != 0)
            {
                /* Choose color based on bin number (cycles through colors) */
                frame[row][col] = (bin % (NUM_COLORS - 1)) + 1; /* Cycle through colors 1-7 (assuming 8 colors) */
            }
            else
            {
                /* No magnitude at this height, set pixel to black */
                frame[row][col] = BLACK;
            }

            /* Check if we've finished drawing this frequency bin and should move to next */
            if((col % BIN_WIDTH_SCREEN) == (BIN_WIDTH_SCREEN - 1))
            {
                if(binMagBuffer[bin] != 0)
                {
                    /* Decrement bin height as we've drawn one row of this bin */
                    binMagBuffer[bin]--;
                }
                /* Move to next frequency bin */
                bin++;
            }
        }
        /* Reset to first frequency bin for the next row */
        bin = 0;
    }
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
void computeFFT(float_t * inputSignal, uint16_t inputSize, float * fftMagBuffer, float * fftFreqBuffer)
{
    float fftInputBuffer[FFT_LEN];   /* Buffer for FFT input */
    float fftOutputBuffer[FFT_LEN];  /* Buffer for FFT output (complex values) */

    /* Copy input signal to FFT input buffer with zero padding */
    for(int index = 0; index < FFT_LEN; index++)
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
    for(int index = 0; index < FFT_LEN / 2; index++)
    {
        /* Calculate magnitude using Pythagorean theorem (sqrt(real² + imag²)) */
        /* Real part is at even indices, imaginary part at odd indices */
        fftMagBuffer[index] = sqrt(
            (fftOutputBuffer[2*index] * fftOutputBuffer[2*index]) +
            (fftOutputBuffer[2*index + 1] * fftOutputBuffer[2*index + 1])
        ) / (FFT_LEN * 1 / 2); /* Normalize magnitude */

        /* Calculate the actual frequency in Hz for this bin */
        fftFreqBuffer[index] = (float)((index * AUDIO_SAMPLE_RATE)) / ((float)FFT_LEN);
    }
}
