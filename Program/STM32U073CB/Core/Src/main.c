/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "capacitive_sensor.h"
#include "LCD_3DIGIT.h"
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

LCD_HandleTypeDef hlcd;

RTC_HandleTypeDef hrtc;

TSC_HandleTypeDef htsc;

/* USER CODE BEGIN PV */

uint32_t HTSC_value_min = 150;
uint32_t HTSC_value_max = 2600;
uint8_t humidity;
char lcd_txt[4] = "123";

Flag system_flag = {0};
State system_state = START;

uint32_t start_timer = 0;
uint32_t now_timer = 0;

volatile uint8_t StopMode_Activated = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_LCD_Init(void);
static void MX_TSC_Init(void);
static void MX_RTC_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// Optimized: Converts int to string without using the heavy sprintf library
void mini_sprintf(int n, char *s, uint8_t width, char pad)
{

	s[width-1] = '\0';
    // 1. Keep track of the sign and use the absolute value
    uint8_t is_negative = (n < 0);
    unsigned int num = is_negative ? -n : n;

    for (int8_t i = width - 2; i >= 0; i--) {
        // 2. Fill digits using the absolute value
        if (num > 0 || (i == width - 2 && num == 0)) {
            s[i] = (num % 10) + '0';
            num /= 10;
        }
        // 3. Place the negative sign once digits are done
        else if (is_negative) {
            s[i] = '-';
            is_negative = 0; // Sign is placed, don't place it again
        }
        // 4. Fill remaining space with padding
        else {
            s[i] = pad;
        }
    }
}

uint8_t update_capa_sensor(uint32_t max_val, uint32_t min_val, TSC_HandleTypeDef *htsc)
{
	uint32_t TSC_val1 = 0;
	uint32_t TSC_val2 = 0;
	int hum = 0;
	char hum_txt[4] = "000";

	TSC_val1 = read_capa_value(htsc);
	if (TSC_val1 == 0) return 200;

	TSC_val2 = read_capa_value(htsc);
	if (TSC_val2 == 0) return 200;

	if (TSC_val2 > TSC_val1 * 1.05 || TSC_val2 < TSC_val1 * 0.95) return 200;

	hum = ((TSC_val1 - min_val)*100)/(max_val - min_val);

	hum = 100 - hum;

	if(hum > 100) hum = 100;
	if(hum < 0) hum = 0;

	mini_sprintf(hum, hum_txt, sizeof(hum_txt), ' ');

	LCD_WriteDigit(hum_txt);

	return hum;
}

uint8_t cal_capa_sensor(uint32_t *max_val, uint32_t *min_val, TSC_HandleTypeDef *htsc)
{
	uint32_t TSC_val1 = 0;
	uint32_t TSC_val2 = 0;
	uint32_t start_time = HAL_GetTick();
	uint32_t now = HAL_GetTick();

	LCD_WriteDigit("cal");
	HAL_Delay(4000);

	LCD_WriteDigit("sec");
	start_time = HAL_GetTick();
	while(HAL_GPIO_ReadPin(CAL_SW_GPIO_Port, CAL_SW_Pin))
	{
		now = HAL_GetTick();
		if(now - start_time > 30000) return 0;
	}

	LCD_WriteDigit("*  ");
	HAL_Delay(1000);
	LCD_WriteDigit(" * ");
	HAL_Delay(1000);
	LCD_WriteDigit("  *");
	HAL_Delay(1000);
	LCD_WriteDigit("  o");
	HAL_Delay(1000);
	LCD_WriteDigit(" o ");
	HAL_Delay(1000);

	TSC_val1 = read_capa_value(htsc);
	if (TSC_val1 == 0) return 0;

	TSC_val2 = read_capa_value(htsc);
	if (TSC_val2 == 0) return 0;

	if (TSC_val2 > TSC_val1 * 1.05 || TSC_val2 < TSC_val1 * 0.95) return 0;

	*max_val = TSC_val1 * 1.05;

	LCD_WriteDigit("o  ");
	HAL_Delay(1000);

	LCD_WriteDigit("ok ");
	HAL_Delay(1000);


	LCD_WriteDigit("eau");
	start_time = HAL_GetTick();
	while(HAL_GPIO_ReadPin(CAL_SW_GPIO_Port, CAL_SW_Pin))
	{
		now = HAL_GetTick();
		if(now - start_time > 30000) return 0;
	}

	TSC_val1 = 0;
	TSC_val2 = 0;

	LCD_WriteDigit("*  ");
	HAL_Delay(1000);
	LCD_WriteDigit(" * ");
	HAL_Delay(1000);
	LCD_WriteDigit("  *");
	HAL_Delay(1000);
	LCD_WriteDigit("  o");
	HAL_Delay(1000);
	LCD_WriteDigit(" o ");
	HAL_Delay(1000);

	TSC_val1 = read_capa_value(htsc);
	if (TSC_val1 == 0) return 0;

	TSC_val2 = read_capa_value(htsc);
	if (TSC_val2 == 0) return 0;

	if (TSC_val2 > TSC_val1 * 1.05 || TSC_val2 < TSC_val1 * 0.95) return 0;

	*min_val = TSC_val1 * 0.95;

	LCD_WriteDigit("o  ");
	HAL_Delay(1000);

	LCD_WriteDigit("ok ");
	HAL_Delay(1000);

	return 1;
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
  MX_LCD_Init();
  MX_TSC_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */
  HAL_LCD_Clear(&hlcd);

  system_flag.need_measurement = 1;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

	  switch (system_state)
	  {
	  case START:
		  if(system_flag.need_calibration)
		  {
			  system_state = CALIBRATION;
		  }
		  else if(system_flag.need_measurement)
		  {
			  system_state = MEASUREMENT;
		  }
		  else if(system_flag.low_humidity)
		  {
			  start_timer = HAL_GetTick();
			  system_state = BLINK_LED;
		  }
		  else
		  {
			  system_state = SLEEP_MODE;
		  }
		  break;

	  case CALIBRATION:
		  if(cal_capa_sensor(&HTSC_value_max, &HTSC_value_min, &htsc))
		  {
			  system_flag.need_calibration = 0;
			  system_flag.need_measurement = 1;
		  }
		  else
		  {
			  system_flag.need_calibration = 1;
		  }

		  if(system_flag.need_calibration)
		  {
			  system_state = CALIBRATION;
		  }
		  else if(system_flag.need_measurement)
		  {
			  system_state = MEASUREMENT;
		  }
		  else if(system_flag.low_humidity)
		  {
			  start_timer = HAL_GetTick();
			  system_state = BLINK_LED;
		  }
		  else
		  {
			  system_state = SLEEP_MODE;
		  }

		  break;

	  case MEASUREMENT:
		  humidity = update_capa_sensor(HTSC_value_max, HTSC_value_min, &htsc);

		  if(humidity == 200)
		  {
			  system_flag.need_measurement = 1;
		  }
		  else if(humidity < 20)
		  {
			  system_flag.need_measurement = 0;
			  system_flag.low_humidity = 1;
		  }
		  else
		  {
			  system_flag.need_measurement = 0;
		  }

		  if(system_flag.need_calibration)
		  {
			  system_state = CALIBRATION;
		  }
		  else if(system_flag.need_measurement)
		  {
			  system_state = MEASUREMENT;
		  }
		  else if(system_flag.low_humidity)
		  {
			  start_timer = HAL_GetTick();
			  system_state = BLINK_LED;
		  }
		  else
		  {
			  system_state = SLEEP_MODE;
		  }

		  break;

	  case BLINK_LED:
		  HAL_GPIO_WritePin(RED_LED_GPIO_Port, RED_LED_Pin, SET);
		  now_timer = HAL_GetTick();

		  if(now_timer - start_timer > 3000) system_flag.low_humidity = 0;

		  if(system_flag.need_calibration)
		  {
			  HAL_GPIO_WritePin(RED_LED_GPIO_Port, RED_LED_Pin, RESET);
			  system_state = CALIBRATION;
		  }
		  else if(system_flag.need_measurement)
		  {
			  HAL_GPIO_WritePin(RED_LED_GPIO_Port, RED_LED_Pin, RESET);
			  system_state = MEASUREMENT;
		  }
		  else if(system_flag.low_humidity)
		  {
			  system_state = BLINK_LED;
		  }
		  else
		  {
			  HAL_GPIO_WritePin(RED_LED_GPIO_Port, RED_LED_Pin, RESET);
			  system_state = SLEEP_MODE;
		  }

		  break;

	  case SLEEP_MODE:

		  HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, 1200-1, RTC_WAKEUPCLOCK_CK_SPRE_16BITS, 0);
		  StopMode_Activated = 1;
		  HAL_SuspendTick();
		  HAL_PWREx_EnterSTOP2Mode(PWR_SLEEPENTRY_WFI);

		  HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);

		  system_flag.need_measurement = 1;

		  if(system_flag.need_calibration)
		  {
			  system_state = CALIBRATION;
		  }
		  else if(system_flag.need_measurement)
		  {
			  system_state = MEASUREMENT;
		  }
		  else if(system_flag.low_humidity)
		  {
			  start_timer = HAL_GetTick();
			  system_state = BLINK_LED;
		  }
		  else
		  {
			  system_state = SLEEP_MODE;
		  }

		  break;

	  default:
		  system_state = START;
		  break;
	  }

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
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief LCD Initialization Function
  * @param None
  * @retval None
  */
static void MX_LCD_Init(void)
{

  /* USER CODE BEGIN LCD_Init 0 */

  /* USER CODE END LCD_Init 0 */

  /* USER CODE BEGIN LCD_Init 1 */

  /* USER CODE END LCD_Init 1 */
  hlcd.Instance = LCD;
  hlcd.Init.Prescaler = LCD_PRESCALER_1;
  hlcd.Init.Divider = LCD_DIVIDER_16;
  hlcd.Init.Duty = LCD_DUTY_1_4;
  hlcd.Init.Bias = LCD_BIAS_1_2;
  hlcd.Init.VoltageSource = LCD_VOLTAGESOURCE_INTERNAL;
  hlcd.Init.Contrast = LCD_CONTRASTLEVEL_3;
  hlcd.Init.DeadTime = LCD_DEADTIME_0;
  hlcd.Init.PulseOnDuration = LCD_PULSEONDURATION_0;
  hlcd.Init.BlinkMode = LCD_BLINKMODE_OFF;
  hlcd.Init.BlinkFrequency = LCD_BLINKFREQUENCY_DIV8;
  hlcd.Init.MuxSegment = LCD_MUXSEGMENT_DISABLE;
  if (HAL_LCD_Init(&hlcd) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LCD_Init 2 */

  /* USER CODE END LCD_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  hrtc.Init.OutPutPullUp = RTC_OUTPUT_PULLUP_NONE;
  hrtc.Init.BinMode = RTC_BINARY_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable the WakeUp
  */
  if (HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, 599, RTC_WAKEUPCLOCK_RTCCLK_DIV16, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief TSC Initialization Function
  * @param None
  * @retval None
  */
static void MX_TSC_Init(void)
{

  /* USER CODE BEGIN TSC_Init 0 */

  /* USER CODE END TSC_Init 0 */

  /* USER CODE BEGIN TSC_Init 1 */

  /* USER CODE END TSC_Init 1 */

  /** Configure the TSC peripheral
  */
  htsc.Instance = TSC;
  htsc.Init.CTPulseHighLength = TSC_CTPH_2CYCLES;
  htsc.Init.CTPulseLowLength = TSC_CTPL_2CYCLES;
  htsc.Init.SpreadSpectrum = DISABLE;
  htsc.Init.SpreadSpectrumDeviation = 1;
  htsc.Init.SpreadSpectrumPrescaler = TSC_SS_PRESC_DIV1;
  htsc.Init.PulseGeneratorPrescaler = TSC_PG_PRESC_DIV4;
  htsc.Init.MaxCountValue = TSC_MCV_8191;
  htsc.Init.IODefaultMode = TSC_IODEF_OUT_PP_LOW;
  htsc.Init.SynchroPinPolarity = TSC_SYNC_POLARITY_FALLING;
  htsc.Init.AcquisitionMode = TSC_ACQ_MODE_NORMAL;
  htsc.Init.MaxCountInterrupt = DISABLE;
  htsc.Init.ShieldIOs = 0;
  htsc.Init.ChannelIOs = TSC_GROUP1_IO1;
  htsc.Init.SamplingIOs = TSC_GROUP1_IO2;
  if (HAL_TSC_Init(&htsc) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_SYSCFG_DisableTSCComparatorMode();
  /* USER CODE BEGIN TSC_Init 2 */

  /* USER CODE END TSC_Init 2 */

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

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(RED_LED_GPIO_Port, RED_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : RED_LED_Pin */
  GPIO_InitStruct.Pin = RED_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(RED_LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : CAL_SW_Pin MES_SW_Pin */
  GPIO_InitStruct.Pin = CAL_SW_Pin|MES_SW_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
/**
  * @brief  EXTI line detection callback.
  * @param  GPIO_Pin Specifies the port pin connected to corresponding EXTI line.
  * @retval None
  */
void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{

  if(StopMode_Activated == 1)
  {
	  SystemClock_Config();
      HAL_ResumeTick();
      StopMode_Activated = 0;
  }

  if(GPIO_Pin == CAL_SW_Pin)
  {
	  system_flag.need_calibration = 1;
  }
  else if(GPIO_Pin == MES_SW_Pin)
  {
	  system_flag.need_measurement = 1;
  }
  else
  {
	  __NOP();
  }
}

void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *hrtc)
{
	SystemClock_Config ();
	HAL_ResumeTick();

	StopMode_Activated = 0;

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
#ifdef USE_FULL_ASSERT
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
