/**
  ******************************************************************************
  * @file    BMS.h
  * @author  Bartosz Rychlicki

  * @Title   Firmware for BMS Master PCB board
  *
  * @brief   This file contains common defines, flags and macros that are used to provide high quality workflow of BMS functionalities.
  *
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 AGH Eko-Energy.
  * All rights reserved.
  *
  ******************************************************************************
  */

#ifndef INC_BMS_H_
#define INC_BMS_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes --------------------------------------------------------------------------------  */
#include "BMS_Types.h"

/* Macros ----------------------------------------------------------------------------------  */
#define BMS_LED_PERIOD (500)															/*< LED blink period in normal/error mode [ms]>*/

/* Variables -------------------------------------------------------------------------------  */
extern uint32_t lastTick;																/*< Last SysTick value used by BMS_Mode_LEDBlink>*/


/* Functions Prototypes --------------------------------------------------------------------  */

/*
	 ==============================================================================
						   ##### INIT / DEINIT #####
	 ==============================================================================
*/

/*
  * @brief  Initializes BMS object, assigns peripheral handles and starts CAN/ADC/PWM/EH
  * @param  bms    Pointer to BMS handle
  * @param  bhcan1 Pointer to CAN1 HAL handle (TX / error reporting)
  * @param  bhcan2 Pointer to CAN2 HAL handle (cell temperatures RX)
  * @param  hadc   Pointer to ADC HAL handle
  * @param  huart  Pointer to UART HAL handle (logger)
  * @param  htim   Pointer to TIM HAL handle (PWM)
  * @retval HAL_OK on success, HAL_ERROR on failure
  */
HAL_StatusTypeDef BMS_Init(BMS_TypeDef* bms,  CAN_HandleTypeDef* bhcan1, CAN_HandleTypeDef* bhcan2, ADC_HandleTypeDef* hadc, UART_HandleTypeDef* huart, TIM_HandleTypeDef* htim);

/*
	 ==============================================================================
						   ##### MODE HANDLERS #####
	 ==============================================================================
*/

/*
  * @brief  Executes BMS normal-mode workflow (ADC, CAN, PWM, HVIL, FAN)
  * @param  bms Pointer to BMS handle
  * @retval HAL_OK on success, HAL_ERROR on failure
  */
HAL_StatusTypeDef BMS_Mode_Normal(BMS_TypeDef* bms);

/*
  * @brief  Executes BMS error-mode workflow (stop peripherals, PWM sleep)
  * @param  bms Pointer to BMS handle
  * @retval HAL_OK on success, HAL_ERROR on failure
  */
HAL_StatusTypeDef BMS_Mode_Error(BMS_TypeDef* bms);

/*
  * @brief  Changes BMS operating mode and stores previous status
  * @param  bms    Pointer to BMS handle
  * @param  status New BMS status to apply
  * @retval HAL_OK on success, HAL_ERROR on failure
  */
HAL_StatusTypeDef BMS_Mode_Change(BMS_TypeDef* bms, BMS_StatusTypeDef_e status);

/*
  * @brief  Logs BMS runtime data (UART / diagnostics placeholder)
  * @param  bms Pointer to BMS handle
  * @retval HAL_OK on success, HAL_ERROR on failure
  */
HAL_StatusTypeDef BMS_Log_Data(BMS_TypeDef* bms);

/*
  * @brief  Blinks status LEDs according to current BMS mode
  * @param  bms Pointer to BMS handle
  * @retval None
  */
void 			  BMS_Mode_LEDBlink(BMS_TypeDef* bms);

/*
	 ==============================================================================
						   ##### PERIPHERALS #####
	 ==============================================================================
*/

/*
  * @brief  Starts / wakes BMS peripherals after leaving error/standby
  * @param  bms Pointer to BMS handle
  * @retval HAL_OK on success, HAL_ERROR on failure
  */
HAL_StatusTypeDef BMS_Start_Peripherals(BMS_TypeDef* bms);

/*
  * @brief  Stops BMS peripherals for error/standby mode
  * @param  bms Pointer to BMS handle
  * @retval HAL_OK on success, HAL_ERROR on failure
  */
HAL_StatusTypeDef BMS_Stop_Peripherals(BMS_TypeDef* bms);

/*
	 ==============================================================================
						   ##### SAFETY / COOLING #####
	 ==============================================================================
*/

/*
  * @brief  Handles HVIL vs safe-state consistency and reports leaks in PROD builds
  * @param  bms Pointer to BMS handle
  * @retval HAL_OK on success, HAL_ERROR on failure
  */
HAL_StatusTypeDef BMS_HVIL_Handler(BMS_TypeDef* bms);

/*
  * @brief  Controls cooling FAN based on maxTemperature hysteresis thresholds
  * @param  bms Pointer to BMS handle
  * @retval HAL_OK on success, HAL_ERROR on failure
  */
HAL_StatusTypeDef BMS_FAN_Control(BMS_TypeDef* bms);

#ifdef __cplusplus
}
#endif

#endif /* INC_BMS_H_ */
