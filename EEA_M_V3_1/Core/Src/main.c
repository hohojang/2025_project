/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  * 전기조명설비 M main.c 파일
  * Author: PARK JANG HO
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
#include "adc.h"
#include "dma.h"
#include "usart.h"
#include "gpio.h"

#include <E220-900T22D.h>
#include <E220_Config.h>
#include <NNFQueue.h>
#include <string.h>
#include <stdio.h>
#include <Sensor.h>
#include "../../Module/EEA_protocol/Inc/protocol.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SENSOR_UPDATE_INTERVAL_MS 2000 ///< 센서 상태를 주기적으로 검사할 간격
/*---------------------------[ 전역 변수 정의 ]---------------------------*/
uint32_t lastSensorProcessTime = 0;
uint8_t rx_byte = 0;

#define RX_BUFFER_SIZE 32
static element rxBuffer[RX_BUFFER_SIZE];
CircularQueue rxQueue;

/**< 마지막으로 송신한 EEA 패킷 */
volatile EEAPacket_t latestTxPacket;
/**< 마지막으로 수신한 EEA 패킷 */
volatile EEAPacket_t latestRxPacket;

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
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
  MX_ADC1_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
  /* ------------------------[ 2. 통신 및 큐 초기화 ]------------------------ */
  bQueueinit(&rxQueue, rxBuffer, RX_BUFFER_SIZE);  ///< 수신용 순환 큐 초기화
  InitLoRaModule();                                 ///< LoRa 모듈 설정 및 통신 초기화
  HAL_Delay(500);                                   ///< LoRa 안정화 대기
  HAL_UART_Receive_IT(&huart3, (uint8_t*)&rx_byte, 1); ///< UART 수신 인터럽트 등록 (1바이트)

  /* ------------------------[ 3. 센서 초기화 ]------------------------ */
  InitAllSensors();                                 ///< PIR / CDS / LED 관련 초기 설정
  lastSensorProcessTime = HAL_GetTick();            ///< 센서 주기 처리 기준 시각 설정

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  /* ------------------------[ 4. 메인 루프 ]------------------------ */
     while (1) {
         uint32_t now = HAL_GetTick();  ///< 현재 시각 갱신

         // (1) 센서 상태 주기적 검사
         if (now - lastSensorProcessTime >= SENSOR_UPDATE_INTERVAL_MS) {
             lastSensorProcessTime = now;
             ProcessAllSensors();       ///< PIR, CDS 상태 검사 및 LED 상태 보고
         }
             ProcessLoRaReception();    ///< LoRa 패킷 파싱 및 명령 처리
      }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
   if (huart->Instance == USART3) {
      bEnqueue(&rxQueue, rx_byte);  ///< 수신 바이트를 큐에 삽입
      ///< 다음 바이트 수신 대기 (인터럽트 재등록)
      HAL_UART_Receive_IT(&huart3, &rx_byte, 1);
   }
}
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
   if (huart == &huart3) {
      HAL_UART_Receive_IT(huart, (uint8_t*) &rx_byte, 1);
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
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

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
