#include "dac_output.h"

uint16_t u16DataBuffer[256];
int halfWord = 0;


void initDAC()
{
	__HAL_RCC_DAC_CLK_ENABLE(); // turn on the DAC clk

	// for DAC channel 1 (PA4)
	DAC -> CR &= ~DAC_CR_TSEL1; // trigger on TIM6 update event
	DAC -> CR |= DAC_CR_TEN1;  // only update DAC on a trigger
	DAC -> CR |= DAC_CR_EN1;   // enable the DAC to start working

	// these params chosen for 256 Hz sampling rate
	// PSC = 125
	// ARR = 1000 (but then this is 3x faster for some reason, so I chose 3k, not 1k)
	// use PSC = 5 and ARR = 145 for 44.1 KHz sampling rate later (maybe multiply ARR by 3 again..?)
	setup_TIM6(125, 10);
	initDataBuffer();
}

void initDataBuffer()
{
	float * waveTable = getTable(1);
	float waveDCOffset;
	for(int i = 0; i < 256; i++) // translate from float to uint16_t
	{
		waveDCOffset = waveTable[i] + 1; // wave table ranges form [-1, 1], so offset to +ve only
		u16DataBuffer[i] = (waveDCOffset / 2) * (float)(MAX_VAL_UINT16_T); // take a ratio of the maximum possible value
	}
	// u16DataBuffer now represents 2^16 different voltage values ranging from [0, Vcc]
}

void setup_TIM6(uint16_t psc, uint16_t arr)
{
	// Turn on the clock for timer 6
	RCC  -> APB1ENR |= RCC_APB1ENR_TIM6EN; // TIM6 clock

	TIM6 -> PSC   = psc - 1;
	TIM6 -> ARR   = arr - 1; // f_tim = fclk / (psc * arr) Hz
	TIM6 -> DIER |= TIM_DIER_UIE; // on timer update, call interrupt
	TIM6 -> CR2 |= TIM_CR2_MMS_1; // Setup TIM6 to enable TRGO on update event

	TIM6 -> CR1 |= TIM_CR1_CEN; // enable the timer

	NVIC_EnableIRQ(TIM6_DAC_IRQn); // enable the update interrupt event for TIM6 and DAC1/DAC2 underrrun error
}

void TIM6_DAC_IRQHandler()
{
	// acknowledge the pending bit for the interrupt
	TIM6 -> SR &= ~TIM_SR_UIF;

	DAC -> DHR12L1 = u16DataBuffer[halfWord]; // fill the DAC's DHR with next 16-bits (lower 4 get chopped off)
	halfWord++;

	if(halfWord == 256) // sent out all bytes of wave, start over
	{
		halfWord = 0;
	}
}
