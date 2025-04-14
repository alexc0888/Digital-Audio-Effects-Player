#include "dac_output.h"

uint16_t DACFrontBuffer[SONG_BUFF_SIZE];
uint16_t DACBackBuffer[SONG_BUFF_SIZE];
dac_buff_q_t DACCircBuff;

void initDAC()
{
	__HAL_RCC_DAC_CLK_ENABLE(); // turn on the DAC clk

	// for DAC channel 1 (PA4)
	DAC -> CR &= ~DAC_CR_TSEL1; // trigger on TIM6 update event
	DAC -> CR |= DAC_CR_TEN1;  // only update DAC on a trigger
	DAC -> CR |= DAC_CR_DMAEN1; // trigger the DMA on TIM6 update event
	DAC -> CR |= DAC_CR_EN1;   // enable the DAC to start working

	initDacBuffer();
	setup_TIM6(44, 47); // for sending ~44,100 samples every second to DAC
	setup_DMA1_S5();
}

void initDacBuffer()
{
	initCircBuff(DAC_BUFFER_Q_SIZE, &(DACCircBuff.buffStatus)); // set up parameters for buffStatus
	for(int sample = 0; sample < SONG_BUFF_SIZE; sample++)
	{
		DACFrontBuffer[sample] = 0;
		DACBackBuffer[sample]  = 0;
		for(int i = 0; i < 16; i++)
		{
			DACCircBuff.DACBufferQ[i][sample] = 0;
		}
	}
	// DACBuffers now represents 2^16 different voltage values ranging from [0, Vcc]
}

void fillDacBuffer(float songBuffer[SONG_BUFF_SIZE])
{
	int16_t min = 0x7fff;
	int16_t signed16Sample; // preserve a signed version of the sample since the DACBuffers are unsigned
	while(DACCircBuff.buffStatus.full); // Wait for the DMA to consume a set of samples and send to the DAC before creating more

	/* CRITICAL SECTION START:
	 * Do not allow DMA to interrupt and consume a new sample set while we are writing
	 */
	NVIC_DisableIRQ(DMA1_Stream5_IRQn);
	for(int sample = 0; sample < SONG_BUFF_SIZE; sample++)
	{
		signed16Sample = (songBuffer[sample] / MAX_AMPLITUDE) * (float) MAX_VAL_INT16_T; // convert float back into int16_t
		DACCircBuff.DACBufferQ[DACCircBuff.buffStatus.wrPtr][sample] = signed16Sample;
		if(signed16Sample < min)
		{
			min = signed16Sample;
		}
	}
	min = (min < 0) ? -1 * min : min; // force found min value to be +ve

	for(int sample = 0; sample < SONG_BUFF_SIZE; sample++)
	{
		DACCircBuff.DACBufferQ[DACCircBuff.buffStatus.wrPtr][sample] = (int16_t) DACCircBuff.DACBufferQ[DACCircBuff.buffStatus.wrPtr][sample] + min; // offset to +ve only for converting to unsigned value
		DACCircBuff.DACBufferQ[DACCircBuff.buffStatus.wrPtr][sample] = DACCircBuff.DACBufferQ[DACCircBuff.buffStatus.wrPtr][sample] >> 4; // we only have 12 bit DAC, so chop lowest 4 bits
	}
	pushCircBuff(&(DACCircBuff.buffStatus));
	NVIC_EnableIRQ(DMA1_Stream5_IRQn);
	/* CRITICAL SECTION END:
	 * DMA may consume now
	 */

}

void setup_TIM6(uint16_t psc, uint16_t arr)
{
	// Turn on the clock for timer 6
	RCC  -> APB1ENR |= RCC_APB1ENR_TIM6EN; // TIM6 clock

	TIM6 -> PSC  = psc - 1;
	TIM6 -> ARR  = arr - 1; // f_tim = fclk / (psc * arr) Hz
	TIM6 -> CR2 |= TIM_CR2_MMS_1; // Setup TIM6 to enable TRGO on update event

	TIM6 -> CR1 |= TIM_CR1_CEN; // enable the timer

	NVIC_EnableIRQ(TIM6_DAC_IRQn); // enable the update interrupt event for TIM6 and DAC1/DAC2 underrrun error
}

void setup_DMA1_S5(void)
{
  __HAL_RCC_DMA1_CLK_ENABLE();

  DMA1_Stream5 -> CR &= ~DMA_SxCR_EN;

  DMA1_Stream5 -> CR  |= (DMA_SxCR_CHSEL_2 | DMA_SxCR_CHSEL_1 | DMA_SxCR_CHSEL_0); // channel 7 - triggering off of DAC1
  DMA1_Stream5 -> CR  |= (DMA_SxCR_MSIZE_0); // memory size is 16-bit per transaction
  DMA1_Stream5 -> CR  |= (DMA_SxCR_PSIZE_0); // peripheral size is 16-bit per transaction
  DMA1_Stream5 -> CR  |= DMA_SxCR_MINC;
  DMA1_Stream5 -> CR  |= DMA_SxCR_CIRC;
  DMA1_Stream5 -> CR  |= DMA_SxCR_DIR_0; // memory to peripheral transfer
  DMA1_Stream5 -> CR  |= DMA_SxCR_DBM;   // enable double buffer mode
  DMA1_Stream5 -> CR  |= DMA_SxCR_TCIE; // Transfer-complete interrupt for double buffer mode


  DMA1_Stream5 -> NDTR = SONG_BUFF_SIZE; // number of 16-bit transactions
  DMA1_Stream5 -> PAR  = (uint32_t) (&(DAC -> DHR12L1));
  DMA1_Stream5 -> M0AR = (uint32_t) DACFrontBuffer;
  DMA1_Stream5 -> M1AR = (uint32_t) DACBackBuffer;

  NVIC_EnableIRQ(DMA1_Stream5_IRQn); // enable the IRQ
  DMA1_Stream5 -> CR  |= DMA_SxCR_EN; // Enable DMA
}

// Consumer - Pop the latest set of samples from DACBufferQ and transmit to the DAC via DMA
void DMA1_Stream5_IRQHandler()
{
	if(DMA1 -> HISR & DMA_HISR_TCIF5)
	{
		DMA1 -> HIFCR |= DMA_HIFCR_CTCIF5; // acknowledge
	}


	if(!DACCircBuff.buffStatus.empty) // check that circular buffer is not empty
	{
		for(int sample = 0; sample < SONG_BUFF_SIZE; sample++)
		{
			if(DMA1_Stream5 -> CR & DMA_SxCR_CT)
			{
				DACFrontBuffer[sample] = DACCircBuff.DACBufferQ[DACCircBuff.buffStatus.rdPtr][sample];
			}
			else
			{
				DACBackBuffer[sample]  = DACCircBuff.DACBufferQ[DACCircBuff.buffStatus.rdPtr][sample];
			}
		}
		popCircBuff(&(DACCircBuff.buffStatus));
	}
}
