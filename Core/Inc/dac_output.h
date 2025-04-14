#ifndef __DAC_OUTPUT__
#define __DAC_OUTPUT__

#include "stm32f4xx_hal.h"
#include "shared_consts.h"
#include "circular_buffer.h"

#define DAC_BUFFER_Q_SIZE 16 // circular buffer can hold 16 sets of samples at a time

typedef struct
{
	uint16_t DACBufferQ[DAC_BUFFER_Q_SIZE][SONG_BUFF_SIZE];
	circ_buff_t buffStatus;
} dac_buff_q_t;

void initDAC();
void initDacBuffer();
void fillDacBuffer(float[SONG_BUFF_SIZE]);

// STM32 Peripheral Settings
void setup_TIM6(uint16_t, uint16_t);
void setup_DMA1_S5();

#endif
