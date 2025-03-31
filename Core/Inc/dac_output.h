#ifndef __DAC_OUTPUT__
#define __DAC_OUTPUT__

#include "stm32f4xx_hal.h"
#include "wave_table.h"

#define MAX_VAL_UINT16_T (1 << 16) - 1

void initDAC();
void initDataBuffer();

// STM32 Peripheral Settings
void setup_TIM6(uint16_t, uint16_t);

#endif
