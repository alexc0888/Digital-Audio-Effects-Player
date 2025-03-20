
#include "matrix_driver.h"

void initMatrix(void)
{
	// ensure all pins are 0 by default
	HAL_GPIO_WritePin(GPIOE, (GPIO_PIN_All & ~(GPIO_PIN_0 | GPIO_PIN_1)), GPIO_PIN_RESET);
}


void shiftRow(uint8_t rowNum)
{
	// transform rowNum into [A ... E]
	GPIOE->ODR = (rowNum) << 8;
	for(int i = 0; i < 64; i++)
	{
		// set desired RGB for pixel
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_7, GPIO_PIN_SET);

		// pulse clock up
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_13, GPIO_PIN_SET);
		// pulse back down
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_13, GPIO_PIN_RESET);
	}
	// done shifting, do latch + OE to display
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_14 | GPIO_PIN_15, GPIO_PIN_SET);
}
