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
#include "stdint.h"

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

/* USER CODE BEGIN PV */
uint8_t contador = 0;

volatile uint8_t cont1 = 0;
volatile uint8_t cont2 = 0;
volatile uint8_t gameover = 0;
volatile uint8_t conteo = 0;
volatile uint8_t segundos = 0;
volatile uint8_t inicio = 0;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

static void Display7Seg_CA(uint8_t digit) // CA = ánodo común (0=enciende)
{
    uint8_t a=0,b=0,c=0,d=0,e=0,f=0,g=0;

    switch(digit)
    {
        case 0: a=1;b=1;c=1;d=1;e=1;f=1;g=0; break;
        case 1: a=0;b=1;c=1;d=0;e=0;f=0;g=0; break;
        case 2: a=1;b=1;c=0;d=1;e=1;f=0;g=1; break;
        case 3: a=1;b=1;c=1;d=1;e=0;f=0;g=1; break;
        case 4: a=0;b=1;c=1;d=0;e=0;f=1;g=1; break;
        case 5: a=1;b=0;c=1;d=1;e=0;f=1;g=1; break;
        default:
            a=b=c=d=e=f=g=0; break;
    }

    HAL_GPIO_WritePin(GPIOB, Disp_a_Pin, a ? GPIO_PIN_RESET : GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, Disp_b_Pin, b ? GPIO_PIN_RESET : GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, Disp_c_Pin, c ? GPIO_PIN_RESET : GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, Disp_d_Pin, d ? GPIO_PIN_RESET : GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, Disp_e_Pin, e ? GPIO_PIN_RESET : GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, Disp_f_Pin, f ? GPIO_PIN_RESET : GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, Disp_g_Pin, g ? GPIO_PIN_RESET : GPIO_PIN_SET);
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
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    /* USER CODE END WHILE */
    /* USER CODE BEGIN 3 */

	  if(conteo == 1)
	  {
		  gameover = 0;
		  HAL_GPIO_TogglePin(LED_Prueba_GPIO_Port, LED_Prueba_Pin);
		  Display7Seg_CA(5 - segundos);
		  HAL_Delay(1000);
		  segundos++;

		  if(segundos > 5)
		  {
			  segundos = 0;
			  conteo = 0;
			  inicio = 1;
		  }
	  }
	  else
	  {
		  HAL_Delay(10); // para no trabar el CPU cuando no está contando
	  }

    /* USER CODE END 3 */
  }
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
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13|Cont1_0_Pin|Cont1_1_Pin|Cont1_2_Pin
                          |Cont1_3_Pin|Cont2_0_Pin|Cont2_1_Pin|Cont2_2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_Prueba_GPIO_Port, LED_Prueba_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, Cont2_3_Pin|Disp_f_Pin|Disp_g_Pin|Disp_a_Pin
                          |Disp_b_Pin|Disp_c_Pin|Disp_d_Pin|Disp_e_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : PC13 Cont1_0_Pin Cont1_1_Pin Cont1_2_Pin
                           Cont1_3_Pin Cont2_0_Pin Cont2_1_Pin Cont2_2_Pin */
  GPIO_InitStruct.Pin = GPIO_PIN_13|Cont1_0_Pin|Cont1_1_Pin|Cont1_2_Pin
                          |Cont1_3_Pin|Cont2_0_Pin|Cont2_1_Pin|Cont2_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : USART_RX_Pin */
  GPIO_InitStruct.Pin = USART_RX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
  HAL_GPIO_Init(USART_RX_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LED_Prueba_Pin */
  GPIO_InitStruct.Pin = LED_Prueba_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_Prueba_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : BTN_Start_Pin BTN1_Pin BTN2_Pin */
  GPIO_InitStruct.Pin = BTN_Start_Pin|BTN1_Pin|BTN2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : Cont2_3_Pin Disp_f_Pin Disp_g_Pin Disp_a_Pin
                           Disp_b_Pin Disp_c_Pin Disp_d_Pin Disp_e_Pin */
  GPIO_InitStruct.Pin = Cont2_3_Pin|Disp_f_Pin|Disp_g_Pin|Disp_a_Pin
                          |Disp_b_Pin|Disp_c_Pin|Disp_d_Pin|Disp_e_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){

	if(GPIO_Pin == BTN1_Pin){

		if(gameover == 0 && inicio == 1){

			cont1++;

			if(cont1 == 1){

				HAL_GPIO_WritePin(GPIOC, Cont1_0_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOC, Cont1_1_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOC, Cont1_2_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOC, Cont1_3_Pin, GPIO_PIN_RESET);

			}

			else if(cont1 == 2){

				HAL_GPIO_WritePin(GPIOC, Cont1_0_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOC, Cont1_1_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOC, Cont1_2_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOC, Cont1_3_Pin, GPIO_PIN_RESET);

			}

			else if(cont1 == 3){

				HAL_GPIO_WritePin(GPIOC, Cont1_0_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOC, Cont1_1_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOC, Cont1_2_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOC, Cont1_3_Pin, GPIO_PIN_RESET);

			}

			else if(cont1 == 4){
				if(cont2 < 4){

					HAL_GPIO_WritePin(GPIOC, Cont1_0_Pin, GPIO_PIN_SET);
					HAL_GPIO_WritePin(GPIOC, Cont1_1_Pin, GPIO_PIN_SET);
					HAL_GPIO_WritePin(GPIOC, Cont1_2_Pin, GPIO_PIN_SET);
					HAL_GPIO_WritePin(GPIOC, Cont1_3_Pin, GPIO_PIN_SET);

					HAL_GPIO_WritePin(GPIOC, Cont2_0_Pin, GPIO_PIN_RESET);
					HAL_GPIO_WritePin(GPIOC, Cont2_1_Pin, GPIO_PIN_RESET);
					HAL_GPIO_WritePin(GPIOC, Cont2_2_Pin, GPIO_PIN_RESET);
					HAL_GPIO_WritePin(GPIOB, Cont2_3_Pin, GPIO_PIN_RESET);

					Display7Seg_CA(2);
					cont1 = 0;
					cont2 = 0;
					gameover = 1;
					inicio = 0;

				}
			}


		}

	}

	else if(GPIO_Pin == BTN2_Pin){

		if(gameover == 0 && inicio == 1){

			cont2++;

			if(cont2 == 1){

				HAL_GPIO_WritePin(GPIOC, Cont2_0_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOC, Cont2_1_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOC, Cont2_2_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOB, Cont2_3_Pin, GPIO_PIN_RESET);

			}

			else if(cont2 == 2){

				HAL_GPIO_WritePin(GPIOC, Cont2_0_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOC, Cont2_1_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOC, Cont2_2_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOB, Cont2_3_Pin, GPIO_PIN_RESET);

			}

			else if(cont2 == 3){

				HAL_GPIO_WritePin(GPIOC, Cont2_0_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOC, Cont2_1_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOC, Cont2_2_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOB, Cont2_3_Pin, GPIO_PIN_RESET);

			}

			else if(cont2 == 4){
				if(cont1 < 4){

					HAL_GPIO_WritePin(GPIOC, Cont1_0_Pin, GPIO_PIN_RESET);
					HAL_GPIO_WritePin(GPIOC, Cont1_1_Pin, GPIO_PIN_RESET);
					HAL_GPIO_WritePin(GPIOC, Cont1_2_Pin, GPIO_PIN_RESET);
					HAL_GPIO_WritePin(GPIOC, Cont1_3_Pin, GPIO_PIN_RESET);

					HAL_GPIO_WritePin(GPIOC, Cont2_0_Pin, GPIO_PIN_SET);
					HAL_GPIO_WritePin(GPIOC, Cont2_1_Pin, GPIO_PIN_SET);
					HAL_GPIO_WritePin(GPIOC, Cont2_2_Pin, GPIO_PIN_SET);
					HAL_GPIO_WritePin(GPIOB, Cont2_3_Pin, GPIO_PIN_SET);


					Display7Seg_CA(1);
					cont1 = 0;
					cont2 = 0;
					gameover = 1;
					inicio = 0;
				}
			}
		}
	}

	else if(GPIO_Pin == BTN_Start_Pin){

		HAL_GPIO_WritePin(GPIOC, Cont1_0_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOC, Cont1_1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOC, Cont1_2_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOC, Cont1_3_Pin, GPIO_PIN_RESET);

		HAL_GPIO_WritePin(GPIOC, Cont2_0_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOC, Cont2_1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOC, Cont2_2_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, Cont2_3_Pin, GPIO_PIN_RESET);

		conteo = 1;
	}
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
