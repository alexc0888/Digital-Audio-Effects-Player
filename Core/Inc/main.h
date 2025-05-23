/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/*
 * main.h
 *
 *  Created on: Mar 20, 2025
 *      Author: alexc0888 (Alex Chitsazzadeh)
 */

/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "shared_consts.h"
#include "matrix_driver.h"
#include "fft_driver.h"
#include "dac_output.h"
#include "my_sdcard.h"
#include "wav_parser.h"
#include "audio_processing.h"
#include "oled_driver.h"
#include "user_inputs.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef enum
{
	TRACK_LIST_STATE,
	OPTIONS_LIST_STATE,
	YES_NO_LIST_STATE,
	TRACK_PLAYING_STATE
} state_t;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void OLED_Reset(void);
void OLED_WriteReg(uint8_t Reg);
void OLED_WriteData(uint8_t Reg);
void OLED_StartWrite();
void OLED_WriteSeq(uint8_t *data, int len);
void OLED_EndWrite();

char listState(char **listToDisp, int numItems, char *header, int *cursor);
void playTrackState(int, char*);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define USER_Btn_Pin GPIO_PIN_13
#define USER_Btn_GPIO_Port GPIOC
#define MCO_Pin GPIO_PIN_0
#define MCO_GPIO_Port GPIOH
#define OLED_DC_Pin GPIO_PIN_12
#define OLED_DC_GPIO_Port GPIOF
#define LD3_Pin GPIO_PIN_14
#define LD3_GPIO_Port GPIOB
#define STLK_RX_Pin GPIO_PIN_8
#define STLK_RX_GPIO_Port GPIOD
#define STLK_TX_Pin GPIO_PIN_9
#define STLK_TX_GPIO_Port GPIOD
#define OLED_CS_Pin GPIO_PIN_14
#define OLED_CS_GPIO_Port GPIOD
#define OLED_RST_Pin GPIO_PIN_15
#define OLED_RST_GPIO_Port GPIOD
#define USB_PowerSwitchOn_Pin GPIO_PIN_6
#define USB_PowerSwitchOn_GPIO_Port GPIOG
#define USB_OverCurrent_Pin GPIO_PIN_7
#define USB_OverCurrent_GPIO_Port GPIOG
#define SD_DET_Pin GPIO_PIN_6
#define SD_DET_GPIO_Port GPIOC
#define USB_SOF_Pin GPIO_PIN_8
#define USB_SOF_GPIO_Port GPIOA
#define USB_VBUS_Pin GPIO_PIN_9
#define USB_VBUS_GPIO_Port GPIOA
#define USB_ID_Pin GPIO_PIN_10
#define USB_ID_GPIO_Port GPIOA
#define USB_DM_Pin GPIO_PIN_11
#define USB_DM_GPIO_Port GPIOA
#define USB_DP_Pin GPIO_PIN_12
#define USB_DP_GPIO_Port GPIOA
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define PB_RIGHT_Pin GPIO_PIN_4
#define PB_RIGHT_GPIO_Port GPIOD
#define PB_RIGHT_EXTI_IRQn EXTI4_IRQn
#define PB_LEFT_Pin GPIO_PIN_5
#define PB_LEFT_GPIO_Port GPIOD
#define PB_LEFT_EXTI_IRQn EXTI9_5_IRQn
#define PB_UP_Pin GPIO_PIN_6
#define PB_UP_GPIO_Port GPIOD
#define PB_UP_EXTI_IRQn EXTI9_5_IRQn
#define PB_DOWN_Pin GPIO_PIN_7
#define PB_DOWN_GPIO_Port GPIOD
#define PB_DOWN_EXTI_IRQn EXTI9_5_IRQn
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
#define LD2_Pin GPIO_PIN_7
#define LD2_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
