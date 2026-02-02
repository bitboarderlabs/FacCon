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

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define ANA_CT_Pin GPIO_PIN_0
#define ANA_CT_GPIO_Port GPIOC
#define NODEID_8_Pin GPIO_PIN_2
#define NODEID_8_GPIO_Port GPIOC
#define NODEID_1_Pin GPIO_PIN_3
#define NODEID_1_GPIO_Port GPIOC
#define ADC1_IN3_THERM_EXT2_Pin GPIO_PIN_3
#define ADC1_IN3_THERM_EXT2_GPIO_Port GPIOA
#define ANA_THERM_ONBOARD_Pin GPIO_PIN_4
#define ANA_THERM_ONBOARD_GPIO_Port GPIOA
#define ANA_THERM_EXT1_Pin GPIO_PIN_5
#define ANA_THERM_EXT1_GPIO_Port GPIOA
#define NODEID_2_Pin GPIO_PIN_6
#define NODEID_2_GPIO_Port GPIOA
#define RS485_NRE_Pin GPIO_PIN_0
#define RS485_NRE_GPIO_Port GPIOB
#define RS485_DE_Pin GPIO_PIN_1
#define RS485_DE_GPIO_Port GPIOB
#define LED_STATUS_T1C1_Pin GPIO_PIN_9
#define LED_STATUS_T1C1_GPIO_Port GPIOE
#define NODEID_4_Pin GPIO_PIN_10
#define NODEID_4_GPIO_Port GPIOE
#define LED_COMM_T1C2_Pin GPIO_PIN_11
#define LED_COMM_T1C2_GPIO_Port GPIOE
#define LED_ALARM_T1C3_Pin GPIO_PIN_13
#define LED_ALARM_T1C3_GPIO_Port GPIOE
#define OUT_0_Pin GPIO_PIN_8
#define OUT_0_GPIO_Port GPIOD
#define OUT_1_Pin GPIO_PIN_10
#define OUT_1_GPIO_Port GPIOD
#define OUT_2_Pin GPIO_PIN_12
#define OUT_2_GPIO_Port GPIOD
#define OUT_3_Pin GPIO_PIN_14
#define OUT_3_GPIO_Port GPIOD
#define OUT_4_Pin GPIO_PIN_6
#define OUT_4_GPIO_Port GPIOC
#define OUT_5_Pin GPIO_PIN_7
#define OUT_5_GPIO_Port GPIOC
#define LCD_BL_T1C4_Pin GPIO_PIN_11
#define LCD_BL_T1C4_GPIO_Port GPIOA
#define LCD_NRST_Pin GPIO_PIN_0
#define LCD_NRST_GPIO_Port GPIOD
#define LCD_AO_Pin GPIO_PIN_1
#define LCD_AO_GPIO_Port GPIOD
#define LCD_NCS_Pin GPIO_PIN_2
#define LCD_NCS_GPIO_Port GPIOD
#define OUT_6_Pin GPIO_PIN_5
#define OUT_6_GPIO_Port GPIOB
#define DIN_1_Pin GPIO_PIN_0
#define DIN_1_GPIO_Port GPIOE
#define DIN_2_Pin GPIO_PIN_1
#define DIN_2_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
