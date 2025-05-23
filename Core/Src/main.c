/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MAX_FILE_NAME_LEN _MAX_LFN + 1
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

SD_HandleTypeDef hsd;
DMA_HandleTypeDef hdma_sdio_rx;
DMA_HandleTypeDef hdma_sdio_tx;

SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart3;

PCD_HandleTypeDef hpcd_USB_OTG_FS;

/* USER CODE BEGIN PV */
volatile int adcDone = TRUE;
volatile uint16_t adcSamples[5];
volatile knobs_t controlKnobs;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_USB_OTG_FS_PCD_Init(void);
static void MX_SDIO_SD_Init(void);
static void MX_SPI1_Init(void);
static void MX_ADC1_Init(void);
/* USER CODE BEGIN PFP */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	adcDone = TRUE;
	// standardize the potentiometer readings as a percentage
	controlKnobs.bassGain     = (float) adcSamples[0] / 4096.0f;
	controlKnobs.bassCutoff   = (float) adcSamples[1] / 4096.0f;
	controlKnobs.tremoloDepth = (float) adcSamples[2] / 4096.0f;
	controlKnobs.trebleGain   = (float) adcSamples[3] / 4096.0f;
	controlKnobs.trebleCutoff = (float) adcSamples[4] / 4096.0f;
}
void OLED_Reset(void) {
	HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_SET);
	HAL_Delay(100);
	HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_RESET);
	HAL_Delay(100);
	HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_SET);
	HAL_Delay(100);
}

void OLED_WriteReg(uint8_t Reg) {
	HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, &Reg, 1, 1000);
	HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_SET);
}

void OLED_WriteData(uint8_t Reg) {
	HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, &Reg, 1, 1000);
	HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_SET);
}

void OLED_StartWrite() {
	HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_RESET);
}

void OLED_WriteSeq(uint8_t *data, int len) {
	HAL_SPI_Transmit(&hspi1, data, len, 10000);
}

void OLED_EndWrite() {
	HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_SET);
}

char listState(char **listToDisp, int numItems, char *header, int *cursor)
{
	*cursor = 0; // clear out the cursor for next render
	char input = 'X';
	char *list[MAX_LIST_PER_MENU]; // we can only display 7 items per menu, so lets fit our list down to this

	for(int item = 0; item < MAX_LIST_PER_MENU; item++)
	{
		list[item] = "\0";
	}
	for(int item = 0; (item < MAX_LIST_PER_MENU) && (item < numItems); item++)
	{
		list[item] = listToDisp[item];
	}

	do
	{
		input = getButton();

		*cursor += (input == 'D') ? 1  :
							(input == 'U') ? -1 : 0;
		if(*cursor < 0)
		{
			*cursor = numItems - 1;
		}
		if(*cursor > numItems - 1)
		{
			*cursor = 0;
		}
		render_list(*cursor, header, list, numItems);
	} while(input != 'R' && input != 'L');

	return input;
}

void playTrackState(int songSelection, char *songName)
{
  // Initialize important variables for playing song
  int songPlaying = TRUE;
  wav_header_t songInfo;
  int sd_status;
  int16_t songBuffer[SONG_BUFF_SIZE]; // .wav file encoded in pcm_s16le
  float   songBufferFlt[SONG_BUFF_SIZE];
  color_t fftFrame [ROW][COL];
  unsigned int bytesRead;
  int currentByte = 0;
  uint32_t cycle = 0;
	char input = 'X';
	adcDone = TRUE;

	// Load up the song and parse the header of the .WAV file
  if(sdLoadSong(songName) != SD_SUCCESS)
  {
  	Error_Handler();
  }
  if(parseWavHeader(&songInfo) != SD_SUCCESS)
  {
  	Error_Handler();
  }
	int totalDuration = songInfo.Subchunk2Size / AUDIO_SAMPLE_RATE;
	int currTime      = 0;

  while(songPlaying)
  {
  	if(adcDone == TRUE)
  	{
  		// start up another ADC transaction to get the next audioParams inside controlKnobs
  		adcDone = FALSE;
  		HAL_ADC_Start_DMA(&hadc1, (uint32_t *) adcSamples, 5);
  	}
  	sd_status = sdReadSong(songBuffer, &bytesRead);
    if(sd_status == SD_FAIL)
    {
    	Error_Handler();
    }
    else if(sd_status == SD_EOF) // fetched last bit of the song, it will end after this iteration
    {
    	songPlaying = FALSE;
    }

    // Render a new "track playing" frame onto the OLED
    // The SPI transactions to the OLED is not done via DMA, so drawing the screen is awfully slow
    // We will only refresh the screen at very infrequent intervals so that audio lag can be mostly avoided
    currentByte += bytesRead;
    if(cycle == 0)
    {
    	currTime = ((double) currentByte / (double) songInfo.Subchunk2Size) * (double) totalDuration;
      render_track_playing(songName, currTime, totalDuration);
    }
    cycle++;
    if(cycle == 1000)
    {
    	cycle = 0;
    }

    input = getButton();
    if(input == 'L')
    {
    	songPlaying = FALSE;
    }


    // create a floating point buffer of the sample for data processing + FFT
    convS16Float(songBuffer, songBufferFlt, TRUE);

    // Perform audio processing algorithms on songBufferFlt...
    applyAudioEffects(songBufferFlt, SONG_BUFF_SIZE, controlKnobs);

    /* Note that the FFT drawing on the screen will be ever so slightly
     * ahead of the audio playback from the DAC. The reason for this is
     * because we have a producer-consumer situation with the audio samples going into the DAC
     * (producer is CPU calling fillDacBuffer which adds a set of samples to the DACBufferQ, while
     * the consumer is the DMA which pops from the DACBufferQ. This allows for maximum flexibility
     * by ensuring two things:
     * 1). The DMA will always have new audio to playback at the specified 44.1 KHz playback frequency
     * 2). The CPU will not be blocked if the DMA is running behind on DMA playback. The CPU can simply
     *     add the next set of samples to the DACBufferQ and move on to reading the next set of samples
     *     from the SD Card.
     *
     * However, there is no producer-consumer situation set up for drawing the FFT to the LED matrix. Doing
     * so combined with the interpolation would result in far too much memory usage. If we weren't limited to a relatively
     * small set of memory, then we could more tightly synchronize the screen and audio. That being said, the desync is
     * hardly noticeable (on the scale of milliseconds) so it really doesn't present a problem.
     * */

    // Compute FFT and draw frames
    // Step 1: Create and store first frame
    computeFFTScreen(songBufferFlt, SONG_BUFF_SIZE, fftFrame);
    storeFrame(fftFrame);
    // Step 2: Generate interpolated frames and display them
    for(int step = 0; step < INTERP_STEPS; step++)
    {
      // Calculate interpolation factor (0.0 to 1.0)
      float factor = (float)step / (float)(INTERP_STEPS - 1);
      // Draw the interpolated frame
      drawInterpFrame(factor);
    }

    // Producer - push the latest set of samples onto the DACBufferQ
    fillDacBuffer(songBufferFlt);
  }
  sdCloseFile();

}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_PCD_Init();
  MX_SDIO_SD_Init();
  MX_FATFS_Init();
  MX_SPI1_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
  // Check SD card details
  DEBUG_PRINTF("\n\rSD Card Information:\r\n");
  DEBUG_PRINTF("Block size         : %lu\r\n", hsd.SdCard.BlockSize);
  DEBUG_PRINTF("Block number       : %lu\r\n", hsd.SdCard.BlockNbr);

  // Try mounting the card...
  if(sdMount() != SD_SUCCESS)
  {
    Error_Handler();
  }

  // point back into the songList array with songPtrs
  // I am doing this so that I can pass these parameters into
  // functions that accept generic char ** args
  char songList[MAX_FILE_NUM][MAX_FILE_NAME_LEN];
  char *songPtrs[MAX_FILE_NUM];
  for(int i = 0; i < MAX_FILE_NUM; i++)
  {
  	songPtrs[i] = songList[i];
  }
  int numSongs;
  char *optionList[3] = {"Bass Boost", "Tremolo Filter", "Treble Boost"};
  char *yesNoList[2] = {"Yes", "No"};
	int selection;
	int currOption;
	state_t oledState = TRACK_LIST_STATE; // always turn on machine with this state
	state_t nextState = oledState;
	char stateInput;

  // grab a list of all the songs currently on-file in the SD card
  if(sdGetFileList(songPtrs, SONG_DIR, &numSongs) != SD_SUCCESS)
  {
  	Error_Handler();
  }
  DEBUG_PRINTF("Printing all the found songs: \r\n");
  for(int i = 0; i < numSongs; i++)
  {
  	DEBUG_PRINTF("%s\r\n", songPtrs[i]);
  }
	// Reset the OLED
  OLED_Reset();
  OLED_InitReg();
  OLED_1in5_rgb_Init();


  while(1)
  {

  	if(oledState == TRACK_LIST_STATE)
  	{
  		stateInput = listState(songPtrs, numSongs, "Track Selection", &selection);
  		nextState = (stateInput == 'R') ? TRACK_PLAYING_STATE : OPTIONS_LIST_STATE;
  	}
  	else if(oledState == OPTIONS_LIST_STATE)
  	{
  		stateInput = listState(optionList, sizeof(optionList) / sizeof(char *), "Options", &selection);
  		currOption = selection; // preserve the current selected option for future use
  		nextState = (stateInput == 'R') ? TRACK_LIST_STATE : YES_NO_LIST_STATE;
  	}
  	else if(oledState == YES_NO_LIST_STATE)
  	{
  		stateInput = listState(yesNoList, sizeof(yesNoList) / sizeof(char *), optionList[currOption], &selection);
  		nextState = OPTIONS_LIST_STATE;
  		if(currOption == 0)
  		{
  			TDsrBassBoost(!selection);
  		}
  		else if(currOption == 1)
  		{
  			TDsrTremoloFilter(!selection);
  		}
  		else if(currOption == 1)
  		{
  			TDsrTrebleBoost(!selection);
  		}
  	}
  	else if(oledState == TRACK_PLAYING_STATE)
  	{
      // Initiate hardware for playing music
    	initMatrix();
    	initFrameBuffers();
    	initDAC();
      playTrackState(selection, songList[selection]);
      // reset DAC and matrix to stop audio and visual playback
      initMatrix();
      initFrameBuffers();
      initDAC();
      nextState = TRACK_LIST_STATE;
  	}
  	oledState = nextState;
  }


	// Reset the OLED
  OLED_Reset();
  if(sdUnmount() != SD_SUCCESS)
  {
  	Error_Handler();
  }


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 384;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 8;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV8;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ENABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 5;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_8;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_56CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_9;
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_10;
  sConfig.Rank = 3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_12;
  sConfig.Rank = 4;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_13;
  sConfig.Rank = 5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief SDIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_SDIO_SD_Init(void)
{

  /* USER CODE BEGIN SDIO_Init 0 */

  /* USER CODE END SDIO_Init 0 */

  /* USER CODE BEGIN SDIO_Init 1 */

  /* USER CODE END SDIO_Init 1 */
  hsd.Instance = SDIO;
  hsd.Init.ClockEdge = SDIO_CLOCK_EDGE_RISING;
  hsd.Init.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;
  hsd.Init.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE;
  hsd.Init.BusWide = SDIO_BUS_WIDE_4B;
  hsd.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
  hsd.Init.ClockDiv = 4;
  /* USER CODE BEGIN SDIO_Init 2 */
  // First init with 1B bus - SD card will not initialize with 4 bits
  hsd.Init.BusWide = SDIO_BUS_WIDE_1B;
  if (HAL_SD_Init(&hsd) != HAL_OK) {
      Error_Handler();
  }

  // Now we can switch to 4 bit mode
  if (HAL_SD_ConfigWideBusOperation(&hsd, SDIO_BUS_WIDE_4B) != HAL_OK) {
      Error_Handler();
  }
  /* USER CODE END SDIO_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief USB_OTG_FS Initialization Function
  * @param None
  * @retval None
  */
static void MX_USB_OTG_FS_PCD_Init(void)
{

  /* USER CODE BEGIN USB_OTG_FS_Init 0 */

  /* USER CODE END USB_OTG_FS_Init 0 */

  /* USER CODE BEGIN USB_OTG_FS_Init 1 */

  /* USER CODE END USB_OTG_FS_Init 1 */
  hpcd_USB_OTG_FS.Instance = USB_OTG_FS;
  hpcd_USB_OTG_FS.Init.dev_endpoints = 6;
  hpcd_USB_OTG_FS.Init.speed = PCD_SPEED_FULL;
  hpcd_USB_OTG_FS.Init.dma_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
  hpcd_USB_OTG_FS.Init.Sof_enable = ENABLE;
  hpcd_USB_OTG_FS.Init.low_power_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.lpm_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.battery_charging_enable = ENABLE;
  hpcd_USB_OTG_FS.Init.vbus_sensing_enable = ENABLE;
  hpcd_USB_OTG_FS.Init.use_dedicated_ep1 = DISABLE;
  if (HAL_PCD_Init(&hpcd_USB_OTG_FS) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_OTG_FS_Init 2 */

  /* USER CODE END USB_OTG_FS_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
  /* DMA2_Stream3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);
  /* DMA2_Stream6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream6_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream6_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */
  __HAL_RCC_GPIOE_CLK_ENABLE();

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LD3_Pin|LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, OLED_CS_Pin|OLED_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USB_PowerSwitchOn_GPIO_Port, USB_PowerSwitchOn_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : USER_Btn_Pin */
  GPIO_InitStruct.Pin = USER_Btn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USER_Btn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : OLED_DC_Pin */
  GPIO_InitStruct.Pin = OLED_DC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(OLED_DC_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD3_Pin LD2_Pin */
  GPIO_InitStruct.Pin = LD3_Pin|LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : OLED_CS_Pin OLED_RST_Pin */
  GPIO_InitStruct.Pin = OLED_CS_Pin|OLED_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_PowerSwitchOn_Pin */
  GPIO_InitStruct.Pin = USB_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(USB_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_OverCurrent_Pin */
  GPIO_InitStruct.Pin = USB_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USB_OverCurrent_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SD_DET_Pin */
  GPIO_InitStruct.Pin = SD_DET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(SD_DET_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PB_RIGHT_Pin PB_LEFT_Pin PB_UP_Pin PB_DOWN_Pin */
  GPIO_InitStruct.Pin = PB_RIGHT_Pin|PB_LEFT_Pin|PB_UP_Pin|PB_DOWN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  // GPIOE [RGB1/RGB2, addr, clk, lat, and oe pins for matrix]
  GPIO_InitStruct.Pin  = GPIO_PIN_All & ~(GPIO_PIN_0 | GPIO_PIN_1);
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  // GPIOA_4 is DAC_OUT1 [analog sound wave output]
  GPIO_InitStruct.Pin  = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/**
  * @brief  Retargets the C library printf function to the USART.
  *   None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART3 and Loop until the end of transmission */
  HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, 0xFFFF);

  return ch;
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
