/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
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
#include "ili9341.h"
#include "Bitmaps.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "Neopixel.h"
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
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim1;
DMA_HandleTypeDef hdma_tim1_ch4_trig_com;

/* USER CODE BEGIN PV */
extern uint16_t FONDO_PARQUEO[];
volatile uint8_t PARQUEO_ADC[4];
/* 0 = libre, 1 = ocupado */
/* 0 = libre, 1 = ocupado */
volatile uint8_t P_CARRO_ROJO = 0;
volatile uint8_t P_CARRO_VERDE = 0;
volatile uint8_t P_CARRO_ROSADO_CLARO = 0;
volatile uint8_t P_CARRO_AMARILLO_CLARO = 0;

/* Conteo de parqueos disponibles */
volatile uint8_t cont_parqueos = 8;

/* Umbral de detección */
#define UMBRAL_LDR 100
volatile uint8_t cont_parqueos_anterior = 255;
float brilloled = 30.0f;
uint8_t neo_estado_anterior = 255;

uint8_t rxCommand = 0;
uint8_t txEstado = 0;
uint8_t byteRecibidoESP = 0;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SPI1_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


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
  MX_SPI1_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */

	LCD_Init();

		LCD_Clear(0x536f);
		LCD_Bitmap(0, 0, 220, 240, FONDO_PARQUEO);
		LCD_Bitmap(0, 0, 150, 20, nombre);
		LCD_Bitmap(228, 0, 80, 50, parqueos_disp);
		//Pruebas de posiciones
		//LCD_Bitmap(13, 53, 38, 61, CARRO_AMARILLO);
		LCD_Bitmap(13, 25, 38, 25, indicador);
		//LCD_Bitmap(67, 53, 38, 61, CARRO_ROSADO);
		LCD_Bitmap(67, 25, 38, 25, indicador);
		//LCD_Bitmap(118, 53, 38, 61, CARRO_CELESTE);
		LCD_Bitmap(118, 25, 38, 25, indicador);
		//LCD_Bitmap(170, 53, 38, 61, CARRO_MORADO);
		LCD_Bitmap(170, 25, 38, 25, indicador);
		//LCD_Bitmap(13, 129, 38, 61, CARRO_ROJO);
		LCD_Bitmap(13, 193, 38, 25, indicador);
		//LCD_Bitmap(67, 129, 38, 61, CARRO_VERDE);
		LCD_Bitmap(67, 193, 38, 25, indicador);
		//LCD_Bitmap(118, 129, 38, 61, CARRO_ROSADO_CLARO);
		LCD_Bitmap(118, 193, 38, 25, indicador);
		//LCD_Bitmap(170, 129, 38, 61, CARRO_AMARILLO_CLARO);
		LCD_Bitmap(170, 193, 38, 25, indicador);
		LCD_Bitmap(228, 53, 80, 136, DISPLAY8);
		HAL_ADC_Start_DMA(&hadc1, (uint32_t*)PARQUEO_ADC, 4);

		/* Todos inician libres: verde */
		setBrightness(30);

		setPixelColor(0, 0, 255, 0);
		setPixelColor(1, 0, 255, 0);
		setPixelColor(2, 0, 255, 0);
		setPixelColor(3, 0, 255, 0);

		pixelShow();

		neo_estado_anterior = 0;

		HAL_I2C_EnableListen_IT(&hi2c1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {
		/* -------------------------------------------------
			   LEER LOS 4 ADC CON DMA
			   ------------------------------------------------- */
			HAL_ADC_Start_DMA(&hadc1, (uint32_t*)PARQUEO_ADC, 4);
			HAL_Delay(20);
			HAL_ADC_Stop_DMA(&hadc1);


			/* Variables para saber qué parqueos cambiaron */
			uint8_t cambio_amarillo = 0;
			uint8_t cambio_rosado = 0;
			uint8_t cambio_verde = 0;
			uint8_t cambio_rojo = 0;


			/* -------------------------------------------------
			   PARQUEO 1 - AMARILLO CLARO
			   ------------------------------------------------- */
			uint8_t nuevo_amarillo = (PARQUEO_ADC[0] < 40) ? 1 : 0;

			if(nuevo_amarillo != P_CARRO_AMARILLO_CLARO)
			{
				P_CARRO_AMARILLO_CLARO = nuevo_amarillo;
				cambio_amarillo = 1;

				if(P_CARRO_AMARILLO_CLARO == 1)
					cont_parqueos--;
				else
					cont_parqueos++;
			}


			/* -------------------------------------------------
			   PARQUEO 2 - ROSADO CLARO
			   ------------------------------------------------- */
			uint8_t nuevo_rosado = (PARQUEO_ADC[1] < UMBRAL_LDR) ? 1 : 0;

			if(nuevo_rosado != P_CARRO_ROSADO_CLARO)
			{
				P_CARRO_ROSADO_CLARO = nuevo_rosado;
				cambio_rosado = 1;

				if(P_CARRO_ROSADO_CLARO == 1)
					cont_parqueos--;
				else
					cont_parqueos++;
			}


			/* -------------------------------------------------
			   PARQUEO 3 - VERDE
			   ------------------------------------------------- */
			uint8_t nuevo_verde = (PARQUEO_ADC[2] < UMBRAL_LDR) ? 1 : 0;

			if(nuevo_verde != P_CARRO_VERDE)
			{
				P_CARRO_VERDE = nuevo_verde;
				cambio_verde = 1;

				if(P_CARRO_VERDE == 1)
					cont_parqueos--;
				else
					cont_parqueos++;
			}


			/* -------------------------------------------------
			   PARQUEO 4 - ROJO
			   ------------------------------------------------- */
			uint8_t nuevo_rojo = (PARQUEO_ADC[3] < UMBRAL_LDR) ? 1 : 0;

			if(nuevo_rojo != P_CARRO_ROJO)
			{
				P_CARRO_ROJO = nuevo_rojo;
				cambio_rojo = 1;

				if(P_CARRO_ROJO == 1)
					cont_parqueos--;
				else
					cont_parqueos++;
			}


			/* -------------------------------------------------
			   ACTUALIZAR NEOPIXEL
			   Libre = verde
			   Ocupado = rojo
			   ------------------------------------------------- */
			uint8_t neo_estado_actual =
					(P_CARRO_AMARILLO_CLARO << 0)
					| (P_CARRO_ROSADO_CLARO << 1)
					| (P_CARRO_VERDE << 2)
					| (P_CARRO_ROJO << 3);

			txEstado = neo_estado_actual;

			if(neo_estado_actual != neo_estado_anterior)
			{
				/* LED 0 - Parqueo amarillo claro */
				if(P_CARRO_AMARILLO_CLARO == 1)
					setPixelColor(0, 255, 0, 0);   // Rojo = ocupado
				else
					setPixelColor(0, 0, 255, 0);   // Verde = libre

				/* LED 1 - Parqueo rosado claro */
				if(P_CARRO_ROSADO_CLARO == 1)
					setPixelColor(1, 255, 0, 0);
				else
					setPixelColor(1, 0, 255, 0);

				/* LED 2 - Parqueo verde */
				if(P_CARRO_VERDE == 1)
					setPixelColor(2, 255, 0, 0);
				else
					setPixelColor(2, 0, 255, 0);

				/* LED 3 - Parqueo rojo */
				if(P_CARRO_ROJO == 1)
					setPixelColor(3, 255, 0, 0);
				else
					setPixelColor(3, 0, 255, 0);

				pixelShow();

				neo_estado_anterior = neo_estado_actual;
			}


			/* -------------------------------------------------
			   REDIBUJAR TFT SOLO SI ESE PARQUEO CAMBIÓ
			   ------------------------------------------------- */

			if(cambio_amarillo)
			{
				if(P_CARRO_AMARILLO_CLARO == 1)
					LCD_Bitmap(170, 129, 38, 61, CARRO_AMARILLO_CLARO);
				else
					FillRect(170, 129, 38, 61, 0x536f);
			}

			if(cambio_rosado)
			{
				if(P_CARRO_ROSADO_CLARO == 1)
					LCD_Bitmap(118, 129, 38, 61, CARRO_ROSADO_CLARO);
				else
					FillRect(118, 129, 38, 61, 0x536f);
			}

			if(cambio_verde)
			{
				if(P_CARRO_VERDE == 1)
					LCD_Bitmap(67, 129, 38, 61, CARRO_VERDE);
				else
					FillRect(67, 129, 38, 61, 0x536f);
			}

			if(cambio_rojo)
			{
				if(P_CARRO_ROJO == 1)
					LCD_Bitmap(13, 129, 38, 61, CARRO_ROJO);
				else
					FillRect(13, 129, 38, 61, 0x536f);
			}


			/* -------------------------------------------------
			   ACTUALIZAR CONTADOR DE PARQUEOS DISPONIBLES
			   ------------------------------------------------- */
			if(cont_parqueos != cont_parqueos_anterior)
			{
				switch(cont_parqueos)
				{
					case 0:
						LCD_Bitmap(228, 53, 80, 136, DISPLAY0);
						break;

					case 1:
						LCD_Bitmap(228, 53, 80, 136, DISPLAY1);
						break;

					case 2:
						LCD_Bitmap(228, 53, 80, 136, DISPLAY2);
						break;

					case 3:
						LCD_Bitmap(228, 53, 80, 136, DISPLAY3);
						break;

					case 4:
						LCD_Bitmap(228, 53, 80, 136, DISPLAY4);
						break;

					case 5:
						LCD_Bitmap(228, 53, 80, 136, DISPLAY5);
						break;

					case 6:
						LCD_Bitmap(228, 53, 80, 136, DISPLAY6);
						break;

					case 7:
						LCD_Bitmap(228, 53, 80, 136, DISPLAY7);
						break;

					case 8:
						LCD_Bitmap(228, 53, 80, 136, DISPLAY8);
						break;
				}

				cont_parqueos_anterior = cont_parqueos;
			}

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

//
//				for (int x = 0; x < 319-42; x++) {
//					int anim = (x/10)%4;
//					// anim 0 1 2 3
//					LCD_Sprite(x, 116-29, 42, 29, link, 4, anim, 0, 0);
//					//V_line( x -1, 100, 50, 0x0DFE);
//					HAL_Delay(15);
//
//				}
//				for (int var = 319-24; var > 0;  var--) {
//					int anim = (var / 10) % 4;
//					LCD_Sprite(var, 100, 24, 30, sonics, 4, anim, 1, 0);
//					V_line(var + 24, 100, 30, 0x0DFE);
//					HAL_Delay(15);
//				}
//
//		for (int x = 0; x < 319-24; x++) {
//			int anim = (x/10)%4;
//			// anim 0 1 2 3
//			LCD_Sprite(x, 100, 24, 30, sonics, 4, anim, 0, 0);
//			V_line( x -1, 100, 30, 0x0DFE);
//			HAL_Delay(15);
//
//		}
//		for (int var = 319-24; var > 0;  var--) {
//			int anim = (var / 10) % 4;
//			LCD_Sprite(var, 100, 24, 30, sonics, 4, anim, 1, 0);
//			V_line(var + 24, 100, 30, 0x0DFE);
//			HAL_Delay(15);
//		}

//		for (int var = 0; var < 319-26;  var++) {
//			 int anim = (var / 5) % 4;
//			LCD_Sprite(var,100,26,16,link,4,anim,0,0);
//			V_line( var -1, 100, 16, 0x4d9e);
//			 HAL_Delay(15);
//		}
//		for (int var = 319-26; var > 0;  var--) {
//					 int anim = (var / 5) % 4;
//					LCD_Sprite(var,100,26,16,link,4,anim,1,0);
//					V_line( var +27, 100, 16, 0x4d9e);
//					 HAL_Delay(15);
//				}
//
//		for (int x = 0; x < 320 - 16; x++) {
//		    int anim2 = (x / 10) % 4;
//		    LCD_Sprite(x,100,16,16,kirbys,4,anim2,0,0);
//		    V_line( x -1, 100, 16, 0x74DA);
//		    HAL_Delay(15);
//		  }
//		  for (int x = 320-16; x > 0; x--) {
//		    int anim2 = (x / 10) % 4;
//		    LCD_Sprite(x,100,16,16,kirbys,4,anim2,1,0);
//		    V_line( x +16, 100, 16, 0x74DA);
//		    HAL_Delay(15);
//		  }




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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 84;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
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
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_8B;
  hadc1.Init.ScanConvMode = ENABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 4;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_9;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_10;
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_12;
  sConfig.Rank = 3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_13;
  sConfig.Rank = 4;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 40;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

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
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
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
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 105-1;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

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
  /* DMA2_Stream4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream4_IRQn);

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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, LCD_RST_Pin|E_Pin|LCD_D1_Pin|F_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LCD_RD_Pin|LCD_WR_Pin|LCD_RS_Pin|LCD_D7_Pin
                          |LCD_D0_Pin|LCD_D2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LCD_CS_Pin|G_Pin|LCD_D6_Pin|A_Pin
                          |B_Pin|C_Pin|D_Pin|LCD_D3_Pin
                          |LCD_D5_Pin|LCD_D4_Pin|SD_SS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LCD_RST_Pin LCD_D1_Pin */
  GPIO_InitStruct.Pin = LCD_RST_Pin|LCD_D1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_RD_Pin LCD_WR_Pin LCD_RS_Pin LCD_D7_Pin
                           LCD_D0_Pin LCD_D2_Pin */
  GPIO_InitStruct.Pin = LCD_RD_Pin|LCD_WR_Pin|LCD_RS_Pin|LCD_D7_Pin
                          |LCD_D0_Pin|LCD_D2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_CS_Pin LCD_D6_Pin LCD_D3_Pin LCD_D5_Pin
                           LCD_D4_Pin SD_SS_Pin */
  GPIO_InitStruct.Pin = LCD_CS_Pin|LCD_D6_Pin|LCD_D3_Pin|LCD_D5_Pin
                          |LCD_D4_Pin|SD_SS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : G_Pin A_Pin B_Pin C_Pin
                           D_Pin */
  GPIO_InitStruct.Pin = G_Pin|A_Pin|B_Pin|C_Pin
                          |D_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : E_Pin F_Pin */
  GPIO_InitStruct.Pin = E_Pin|F_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void HAL_I2C_AddrCallback(
    I2C_HandleTypeDef *hi2c,
    uint8_t TransferDirection,
    uint16_t AddrMatchCode)
{
    if(hi2c->Instance == I2C1)
    {
        if(TransferDirection == I2C_DIRECTION_TRANSMIT)
        {
            HAL_I2C_Slave_Seq_Receive_IT(
                hi2c,
                &rxCommand,
                1,
                I2C_LAST_FRAME
            );
        }

        if(TransferDirection == I2C_DIRECTION_RECEIVE)
        {
            HAL_I2C_Slave_Seq_Transmit_IT(
                hi2c,
                &txEstado,
                1,
                I2C_LAST_FRAME
            );
        }
    }
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if(hi2c->Instance == I2C1)
    {
        byteRecibidoESP = rxCommand;
    }
}

void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c)
{
    HAL_I2C_EnableListen_IT(&hi2c1);
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    HAL_I2C_EnableListen_IT(&hi2c1);
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
	while (1) {
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
