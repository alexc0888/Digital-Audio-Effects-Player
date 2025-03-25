#ifndef __MATRIX_DRIVER__
#define __MATRIX_DRIVER__

#include "stm32f4xx_hal.h"

// 32 x 64 LED matrix
#define ROW 32
#define COL 64

// screen data structure only needs half the rows, but needs twice the cols to allow room for clk pin to change
#define SCREEN_ROW ROW / 2
#define SCREEN_COL COL * 2

#define NUM_COLORS 8

// structure for DMA output (16 bits/ 2B)
typedef struct
{
	unsigned int dead:  2;
	unsigned int rgb1:  3;
	unsigned int rgb2:  3;
	unsigned int addr:  5;
	unsigned int oe:    1;
	unsigned int lat:   1;
	unsigned int clk:   1;
} hub75_gpio_t;

// Colors enumerated as 0bRBG
typedef enum
{
	BLACK,
	RED,
	BLUE,
	PURPLE,
	GREEN,
	YELLOW,
	TEAL,
	WHITE
} color_t;

// Driver Functions
void initMatrix(void);
void drawFrame(color_t [ROW][COL]);
void updateRow(uint8_t, color_t [COL]);
void updatePixel(uint8_t, uint8_t, color_t);
void initScreen(void);
uint8_t transformRowNum(uint8_t);

// STM32 Peripheral Settings
void setup_TIM8(uint8_t, uint8_t);
void setup_DMA2_S1(void);

#endif
