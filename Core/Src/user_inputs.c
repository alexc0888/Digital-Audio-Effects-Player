#include "user_inputs.h"

volatile char button = 'X';
char buttonCandidate;

// Interrupt Handlers
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	setup_TIM7(960, 1000); // wait an additional 10 ms for button to stop bouncing by triggering this timer callback
	if(GPIO_Pin == PB_RIGHT_Pin)
	{
		buttonCandidate = 'R';
	}
	else if(GPIO_Pin == PB_LEFT_Pin)
	{
		buttonCandidate = 'L';
	}
	else if(GPIO_Pin == PB_UP_Pin)
	{
		buttonCandidate = 'U';
	}
	else if(GPIO_Pin == PB_DOWN_Pin)
	{
		buttonCandidate = 'D';
	}
	else
	{
		DEBUG_PRINTF("ERROR: EXTI Interrupt triggered by unknown source! Please double-check interrupt configuration settings for the project.");
		Error_Handler();
	}
}

char getButton()
{
	char temp = button;
	button = 'X'; // consumed button, so set it back to X
	return temp;
}

// get new inputs from control knobs
void getKnobs(ADC_HandleTypeDef * adc, knobs_t * controlKnobs)
{
	// Not any real reason to not use polling here since its a rather short task compared to other work CPU needs to do per iteration
	HAL_ADC_Start(adc);
	HAL_ADC_PollForConversion(adc, 10);
	controlKnobs -> bassGain = HAL_ADC_GetValue(adc);
	HAL_ADC_PollForConversion(adc, 10);
	controlKnobs -> bassCutoff = HAL_ADC_GetValue(adc);
	HAL_ADC_Stop(adc);

	// normalize all inputs down to a scale of 1
	controlKnobs -> bassGain /= 4096;
	controlKnobs -> bassCutoff /= 4096;
}


void setup_TIM7(uint16_t psc, uint16_t arr)
{
	// Turn on the clock for timer 7
	RCC  -> APB1ENR |= RCC_APB1ENR_TIM7EN; // TIM7 clock

	TIM7 -> CR1  &= ~TIM_CR1_CEN; // Disable the timer
	TIM7 -> PSC  = psc - 1;
	TIM7 -> ARR  = arr - 1; // f_tim = fclk / (psc * arr) Hz
	TIM7 -> DIER |= TIM_DIER_UIE;
	TIM7 -> CR1  |= TIM_CR1_CEN; // enable the timer

	NVIC_EnableIRQ(TIM7_IRQn); // enable the update interrupt event for TIM7
}

void TIM7_IRQHandler()
{
	// ack the pending bit
	if(TIM7 -> SR & TIM_SR_UIF)
	{
		TIM7 -> SR &= ~TIM_SR_UIF;
	}

	// only update the button press if the candidateButton is still low (meaning it has been pressed, and is done bouncing)
	switch(buttonCandidate)
	{
		case 'R':
			if(HAL_GPIO_ReadPin(PB_RIGHT_GPIO_Port, PB_RIGHT_Pin) == GPIO_PIN_RESET)
			{
				button = buttonCandidate;
			}
			break;

		case 'L':
			if(HAL_GPIO_ReadPin(PB_LEFT_GPIO_Port, PB_LEFT_Pin) == GPIO_PIN_RESET)
			{
				button = buttonCandidate;
			}
			break;

		case 'U':
			if(HAL_GPIO_ReadPin(PB_UP_GPIO_Port, PB_UP_Pin) == GPIO_PIN_RESET)
			{
				button = buttonCandidate;
			}
			break;

		case 'D':
			if(HAL_GPIO_ReadPin(PB_DOWN_GPIO_Port, PB_DOWN_Pin) == GPIO_PIN_RESET)
			{
				button = buttonCandidate;
			}
			break;
	}
	// disable TIM7 so its not called again until another button press is detected
	TIM7 -> CR1 &= ~TIM_CR1_CEN;
}
