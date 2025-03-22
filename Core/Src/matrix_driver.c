
#include "matrix_driver.h"
#include <string.h>


// structure to contain data of what screen is displaying
hub75_gpio_t screen [ROW][COL];
volatile uint16_t temp = 0;
volatile uint8_t curr_row = 0;
volatile uint8_t curr_col = 0;

void initMatrix(void)
{
	// ensure all RGB/Addr pins are 0 by default
	HAL_GPIO_WritePin(GPIOE, (GPIO_PIN_All & ~(GPIO_PIN_0 | GPIO_PIN_1)), GPIO_PIN_RESET);
	// ensure clk, latch, and OE pins are 0 by default
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3 | GPIO_PIN_4, GPIO_PIN_RESET); // latch and oe
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET); // clk

	// clear structure of any lingering data
	memset(screen, 0, sizeof(screen));
	initScreenAddr();
	fillScreen(1); // fill purple


	setup_DMA();
	setup_TIM8();
	DMA2_Stream1 -> CR |= DMA_SxCR_EN;
}

void fillScreen(uint8_t color)
{
	for(int row = 0; row < ROW; row++)
	{
		for(int col = 0; col < COL; col++)
		{
			screen[row][col].rgb1 = color;
			screen[row][col].rgb2 = color;
		}
	}
}

void initScreenAddr()
{
	for(uint8_t row = 0; row < ROW; row++)
	{
		for(uint8_t col = 0; col < COL; col++)
		{
			screen[row][col].addr = row;
		}
	}
}

void setup_TIM8(void)
{
	  // Turn on the clock for timer 8
	  RCC->APB2ENR |= RCC_APB2ENR_TIM8EN;
	  // set the clk freq
	  TIM8->PSC = 100-1;
	  TIM8->ARR = 200-1;
	  // Set for Upcounting
	  TIM8->CR1 &= ~TIM_CR1_DIR;
	  // Set the Channel 1 for PWM mode 2
	  TIM8->CCMR1 |= (TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2| TIM_CCMR1_OC1M_0);
	  // set the CCR value to produce PWM of desired duty cycle
	  TIM8->CCR1 = 100; // 50% duty cycle
	  // Enable DMA transfer at the @posedge of the clk
	  TIM8->DIER |= TIM_DIER_CC1IE;
	  TIM8->DIER |= TIM_DIER_UDE;
	  TIM8->CCER |= TIM_CCER_CC1E;
	  TIM8->BDTR |= TIM_BDTR_MOE;
	  HAL_NVIC_EnableIRQ(TIM8_CC_IRQn);
	  // Turn on timer
	  TIM8->CR1 |= 0x1;
}

void setup_DMA(void)
{
  __HAL_RCC_DMA2_CLK_ENABLE();

  DMA2_Stream1 -> CR &= ~DMA_SxCR_EN;

  DMA2_Stream1 -> CR |= (DMA_SxCR_CHSEL_2 | DMA_SxCR_CHSEL_1 | DMA_SxCR_CHSEL_0); // channel 7
  // Total size of transfer 64 cols
  DMA2_Stream1 -> NDTR = 1;
  // Memory Location
  DMA2_Stream1 -> PAR = (uint32_t) &temp; // source in mem2mem
  DMA2_Stream1 -> M0AR  = (uint32_t) (&(GPIOE->ODR)); // dest in mem2mem
  // periph access : 16 bits
  // mem access : 16 bits
  DMA2_Stream1 -> CR |= (DMA_SxCR_MSIZE_0);
  DMA2_Stream1 -> CR |= (DMA_SxCR_PSIZE_0);
//  DMA2_Stream1 -> CR |= DMA_SxCR_PINC;
//  DMA2_Stream1 -> CR |= DMA_SxCR_CIRC;
  DMA2_Stream1 -> CR |= DMA_SxCR_DIR_1; // mem to mem

//  DMA2_Stream2 -> CR |= DMA_SxCR_TCIE;
//  HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);
//  HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 0, 0);
}

//void DMA2_Stream2_IRQHandler(void) {
//
//  DMA2_Stream2->CR &= ~DMA_SxCR_EN;
//  // acknowledge pending bit
//  DMA2 -> LIFCR |= DMA_LIFCR_CTCIF2;
//  GPIOA -> BSRR |= GPIO_BSRR_BS4; // latch
////	  GPIOA -> BSRR |= GPIO_BSRR_BS3; // OE
//
//  curr_row++;
//  if(curr_row > 15)
//  {
//    curr_row = 0;
//  }
//
//	  DMA2_Stream2->PAR = (uint32_t) (screen + (curr_row*64*2));
//	  GPIOA -> BSRR |= GPIO_BSRR_BR4; // latch
////	  GPIOA -> BSRR |= GPIO_BSRR_BR3; // OE
//  DMA2_Stream2->CR |= DMA_SxCR_EN;
//
//}

void TIM8_CC_IRQHandler(void)
{
	TIM8->SR &= ~TIM_SR_CC1IF;
	temp = screen[curr_row][curr_col].dead1 | (screen[curr_row][curr_col].rgb1 << 2) | (screen[curr_row][curr_col].rgb2 << 5) | (screen[curr_row][curr_col].addr << 8 | (screen[curr_row][curr_col].dead2 << 13));
	DMA2 -> LIFCR |= DMA_LIFCR_CTCIF2;
	DMA2_Stream1 -> CR |= DMA_SxCR_EN;
	curr_col++;
	if(curr_col > 63)
	{
		GPIOA -> BSRR |= GPIO_BSRR_BS4; // activate latch
//		GPIOA -> BSRR |= GPIO_BSRR_BS3; // OE
		curr_row++;
		if(curr_row > 15)
		{
			curr_row = 0;
		}
		curr_col = 0;
		GPIOA -> BSRR |= GPIO_BSRR_BR4;
//		GPIOA -> BSRR |= GPIO_BSRR_BR3; // OE
	}

}


