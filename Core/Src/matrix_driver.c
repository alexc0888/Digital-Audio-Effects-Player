
#include "matrix_driver.h"
#include <string.h>


// structure to contain data of what screen is displaying
hub75_gpio_t screen [ROW][COL];
volatile uint8_t curr_row = 0;

void initMatrix(void)
{
	// ensure all pins are 0 by default
	HAL_GPIO_WritePin(GPIOE, (GPIO_PIN_All & ~(GPIO_PIN_0 | GPIO_PIN_1)), GPIO_PIN_RESET);
	// clear structure of any lingering data
	memset(screen, 1, sizeof(screen));

	setup_TIM2();
	setup_DMA();
}

void setup_TIM2(void)
{
  // Turn on the clock for timer 2
  RCC -> APB1ENR |= RCC_APB1ENR_TIM2EN;
  // set the clk freq
  TIM2->PSC = 10-1;
  TIM2->ARR = 2-1;
  // Set for Upcounting
  TIM2->CR1 &= ~TIM_CR1_DIR;
  // Set the Channel 1 for PWM mode 2
  TIM2->CCMR1 |= (TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2| TIM_CCMR1_OC1M_0);
  // set the CCR value to produce PWM of desired duty cycle
  TIM2->CCR1 = 1;
  // Enable DMA transfer at the @posedge of the clk
  TIM2->DIER |= TIM_DIER_CC1DE;
  TIM2->CCER |= TIM_CCER_CC1E;
  // Turn on timer
  TIM2->CR1 |= 0x1;
}

void setup_DMA(void)
{
  __HAL_RCC_DMA1_CLK_ENABLE();
  // set CCR to reset value;
  DMA1_Stream5 -> CR &= ~DMA_SxCR_EN;
  DMA1_Stream5 -> CR |= (DMA_SxCR_CHSEL_1 | DMA_SxCR_CHSEL_0) ;
  // Total size of transfer 16 rows * 64 cols
  DMA1_Stream5 -> NDTR = COL;
  // Memory Location
  DMA1_Stream5-> M0AR = (uint32_t) screen;
  // Peripheral Destination
  DMA1_Stream5-> PAR = (uint32_t) (&(GPIOE->ODR));
  // periph access : 32 bits
  // mem access : 16 bits
  DMA1_Stream5 -> CR |= (DMA_SxCR_MSIZE_0);
  DMA1_Stream5 -> CR |= (DMA_SxCR_PSIZE_1);
  DMA1_Stream5 -> CR |= DMA_SxCR_MINC;
  DMA1_Stream5 -> CR |= DMA_SxCR_CIRC;
  DMA1_Stream5 -> CR |= DMA_SxCR_DIR_0;

  DMA1_Stream5 -> CR |= DMA_SxCR_TCIE;
  DMA1_Stream5 -> CR |= DMA_SxCR_EN;

  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
}

void DMA1_Stream5_IRQHandler(void) {
  DMA1_Stream5->CR &= ~0x1;
  // acknowledge pending bit
  GPIOA -> BSRR |= GPIO_BSRR_BS4; // latch
  GPIOA -> BSRR |= GPIO_BSRR_BS3; // OE

  curr_row++;
  if(curr_row > 15){
	  curr_row = 0;
  }

  DMA1_Stream5->M0AR = (uint32_t) (screen + (curr_row*64*2));
  GPIOA -> BSRR |= GPIO_BSRR_BR4; // latch
  GPIOA -> BSRR |= GPIO_BSRR_BR3; // OE

  DMA1_Stream5->CR |= 0x1;
}


