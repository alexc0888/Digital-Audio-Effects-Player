#ifndef __MATRIX_DRIVER__
#define __MATRIX_DRIVER__

#include "stm32f4xx_hal.h"

#define ROW 16
#define COL 64
#define MEMSIZE ROW * COL

// structure for DMA output (16 bits/ 2B)
typedef struct
{
	unsigned int dead1: 2;
	unsigned int rgb1:  3;
	unsigned int rgb2:  3;
	unsigned int addr:  5;
	unsigned int dead2: 3;
} hub75_gpio_t;


void initMatrix(void);
void setup_TIM8(void);
void setup_DMA(void);
void fillScreen(uint8_t);
void initScreenAddr();

#endif
