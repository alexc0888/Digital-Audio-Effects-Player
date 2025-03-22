
#include "matrix_driver.h"
#include <string.h>


// structure to contain frame data ready to be directly DMA'ed to matrix pins
hub75_gpio_t screen [SCREEN_ROW][SCREEN_COL];

void initMatrix(void)
{
	// ensure all matrix GPIO  pins are 0 by default
	HAL_GPIO_WritePin(GPIOE, (GPIO_PIN_All & ~(GPIO_PIN_0 | GPIO_PIN_1)), GPIO_PIN_RESET);

	// clear structure of any lingering data
	initScreen();
	fillScreen(3, 6); // purple and teal


	setup_DMA2_S1();
	setup_TIM8(1, 2);
}

void fillScreen(uint8_t color1, uint8_t color2)
{
	for(int row = 0; row < SCREEN_ROW; row++)
	{
		for(int col = 0; col < SCREEN_COL; col++)
		{
			screen[row][col].rgb1 = color1;
			screen[row][col].rgb2 = color2;
		}
	}
}

void initScreen()
{
	for(int row = 0; row < SCREEN_ROW; row++)
	{
		for(int col = 0; col < SCREEN_COL; col++)
		{
			screen[row][col].dead = 0;
			screen[row][col].rgb1 = 0;
			screen[row][col].rgb2 = 0;
			screen[row][col].addr = row;

			// setup clk to automatically rise and fall over each DMA transaction
			if(col % 2)
			{
				screen[row][col].clk = 0;
			}
			else
			{
				screen[row][col].clk = 1;
			}

			// once the final pixel is shifted in, latch it and raise oe
			if(col >= (SCREEN_COL - 2))
			{
				screen[row][col].lat = 1;
				screen[row][col].oe  = 1;
			}
			else
			{
				screen[row][col].lat = 0;
				screen[row][col].oe  = 0;
			}
		}
	}
}

void setup_TIM8(uint8_t psc, uint8_t arr)
{
	// Turn on the clock for timer 8
	RCC  -> APB2ENR |= RCC_APB2ENR_TIM8EN; // TIM8 clock
	TIM8 -> CR1     &= ~TIM_CR1_DIR; // count up

	TIM8 -> PSC   = psc - 1;
	TIM8 -> ARR   = arr - 1; // f_tim = fclk / (psc * arr) Hz
	TIM8 -> DIER |= TIM_DIER_UDE; // on timer update, trigger the DMA
	TIM8 -> CR1  |= TIM_CR1_CEN; // enable the timer
}

void setup_DMA2_S1(void)
{
  __HAL_RCC_DMA2_CLK_ENABLE();

  DMA2_Stream1 -> CR &= ~DMA_SxCR_EN;

  DMA2_Stream1 -> CR  |= (DMA_SxCR_CHSEL_2 | DMA_SxCR_CHSEL_1 | DMA_SxCR_CHSEL_0); // channel 7 - triggering off of TIM8_UP
  DMA2_Stream1 -> CR  |= (DMA_SxCR_MSIZE_1); // memory size is 32-bit per transaction
  DMA2_Stream1 -> CR  |= (DMA_SxCR_PSIZE_1); // peripheral size is 32-bit per transaction
  DMA2_Stream1 -> CR  |= DMA_SxCR_MINC;
  DMA2_Stream1 -> CR  |= DMA_SxCR_CIRC;
  DMA2_Stream1 -> CR  |= DMA_SxCR_DIR_0; // memory to peripheral transfer

  DMA2_Stream1 -> NDTR = SCREEN_ROW * SCREEN_COL; // 2048 transfers
  DMA2_Stream1 -> PAR  = (uint32_t) (&(GPIOE->ODR));
  DMA2_Stream1 -> M0AR = (uint32_t) screen;
  DMA2_Stream1 -> CR  |= DMA_SxCR_EN; // Enable DMA
}


