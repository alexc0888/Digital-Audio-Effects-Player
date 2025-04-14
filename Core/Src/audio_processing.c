#include "audio_processing.h"

// Map the PCM_s16 samples into floating point numbers, on a scale of 0 to 3.3V
// Easier to process and perform FFT on floating point format
void convS16Float(int16_t songBuffer[SONG_BUFF_SIZE], float songBufferFlt[SONG_BUFF_SIZE], int toFloat)
{
	if(toFloat == TRUE)
	{
		for(int sample = 0; sample < SONG_BUFF_SIZE; sample++)
		{
			songBufferFlt[sample] = ((float) songBuffer[sample] / (float) MAX_VAL_INT16_T) * MAX_AMPLITUDE; // take as a fraction of 3.3V
		}
	}
	else // float -> int16_t
	{
		for(int sample = 0; sample < SONG_BUFF_SIZE; sample++)
		{
			songBuffer[sample] = (songBufferFlt[sample] / MAX_AMPLITUDE) * (float) MAX_VAL_INT16_T; // take as a fraction of 3.3V
		}
	}
}
