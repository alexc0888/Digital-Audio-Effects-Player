#ifndef __DAC_OUTPUT__
#define __DAC_OUTPUT__

#include "stm32f4xx_hal.h"
#include "wave_table.h"
#include "my_sdcard.h"

#define MAX_VAL_UINT16_T 0xffff
#define MAX_VAL_INT16_T  0x7fff

void initDAC();
void initDacBuffer();
void fillDacBuffer();

// STM32 Peripheral Settings
void setup_TIM6(uint16_t, uint16_t);


extern volatile int dacDONE;

#endif
