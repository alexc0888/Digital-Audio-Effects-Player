#include "fft_driver.h"


arm_rfft_fast_instance_f32 fftHandler;


void initFFT(void)
{
	arm_rfft_fast_init_f32(&fftHandler, FFT_LEN);
}


void computeFFTScreen(float * inputSignal, uint16_t inputSize, color_t frame[ROW][COL])
{
	float fftMagBuffer[FFT_LEN / 2];
	float fftFreqBuffer[FFT_LEN / 2]; // we only care about frequencies below nyquist rate, so discard second half

	int binMagBuffer[FFT_LEN / 2];
	uint8_t bin;
	computeFFT(inputSignal, inputSize, fftMagBuffer, fftFreqBuffer);

	for(bin = 0; bin < FFT_LEN / 2; bin++)
	{
		binMagBuffer[bin] = round(fftMagBuffer[bin] * (float) ROW); // map each bin to [0, 32] to indicate bin height (0 means no height)
	}
	bin = 0; // restart at bin 0 in the upcoming loop

	for(int row = ROW - 1; row >= 0; row--)
	{
		for(int col = 0; col < COL; col++)
		{
			if(binMagBuffer[bin] != 0)
			{
				frame[row][col] = (bin % (NUM_COLORS - 1)) + 1; // choose a color from RED - WHITE (0b001 -> 0b111) ignore black/blank
			}
			else
			{
				frame[row][col] = BLACK;
			}

			if((col % BIN_WIDTH_SCREEN) == (BIN_WIDTH_SCREEN - 1)) // move to next bin every 4 columns (defining bin width as 4 columns on led matrix)
			{
				if(binMagBuffer[bin] != 0) // -1 indicates that that bin is finished, don't roll over below that
				{
					binMagBuffer[bin]--; // finished a row for this particular bin, so mark it down
				}
				bin++;
			}
		}
		bin = 0; // reset bin for next row
	}
}


void computeFFT(float_t * inputSignal, uint16_t inputSize, float * fftMagBuffer, float * fftFreqBuffer)
{
	float fftInputBuffer[FFT_LEN];
	float fftOutputBuffer[FFT_LEN];

	// fill the input buffer
	for(int index = 0; index < FFT_LEN; index++)
	{
		if(index < inputSize)
		{
			fftInputBuffer[index] = inputSignal[index];
		}
		else
		{
			fftInputBuffer[index] = 0;
		}
	}

	arm_rfft_fast_f32(&fftHandler, fftInputBuffer, fftOutputBuffer, 0);

	for(int index = 0; index < FFT_LEN / 2; index++)
	{
		fftMagBuffer[index]  = sqrt((fftOutputBuffer[2*index] * fftOutputBuffer[2*index]) + (fftOutputBuffer[2*index + 1] * fftOutputBuffer[2*index + 1]))
													 / (FFT_LEN * MAX_AMPLITUDE / 2);
		fftFreqBuffer[index] = (float) ((index * AUDIO_SAMPLE_RATE)) / ((float) FFT_LEN);
	}
}
