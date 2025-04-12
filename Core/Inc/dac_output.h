#ifndef __DAC_OUTPUT__
#define __DAC_OUTPUT__

#include "stm32f4xx_hal.h"
#include "shared_consts.h"

void initDAC();
void initDacBuffer();
void fillDacBuffer();

// STM32 Peripheral Settings
void setup_TIM6(uint16_t, uint16_t);


extern volatile int dacDONE;

#endif
