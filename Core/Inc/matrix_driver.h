#ifndef __MATRIX_DRIVER__
#define __MATRIX_DRIVER__

#include "stm32f4xx_hal.h"

#define ROW 16
#define COL 64
#define MEMSIZE ROW * 64

// structure for DMA output (16 bits/ 2B)
typedef struct
{
	int dead1: 2;
	int rgb1: 3;
	int rgb2: 3;
	int addr: 5;
	int dead2: 3;
} hub75_gpio_t;


void initMatrix(void);
void setup_TIM2(void);
void setup_DMA(void);

#endif
