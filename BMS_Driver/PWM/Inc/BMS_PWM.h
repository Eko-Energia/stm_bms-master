/**
  ******************************************************************************
  * @file    BMS_PWM.h
  * @author  Bartosz Rychlicki

  * @Title   Typedefs and inclues for BMS's PWM module
  *
  * @brief   This file contains typedefs definitions and includes that are required for correct signal generation of PWM.
  *
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 AGH Eko-Energy.
  * All rights reserved.
  *
  ******************************************************************************
  */
#pragma once

#ifndef BMS_PWM_H_
#define BMS_PWM_H_

#ifdef __cplusplus
extern "C" {
#endif


/* Includes --------------------------------------------------------------------------------  */
#include "BMS_Types.h"

/* Macros  --------------------------------------------------------------------------------  */
//< ------------------------------------- COMMON -------------------------------------------
#define TIM_CH3 		       (TIM_CHANNEL_3)  //< Output channel of timer for PWM generation
#define STARTUP_PERIOD         (2000)			//< Period of startup stage for BMS				        [ms]


//< --------------------------------- STARTUP STAGE (2s) ----------------------------------
#define RELAY_STARTUP_DUTY     (100)			//< Duty cycle for generated signal at startup          [%]
#define RELAY_STARTUP_FREQ     (10000)			//< Frequency of generated signal at startup mode       [Hz]

//< --------------------------------- OPERATIONAL STAGE -----------------------------------
#define RELAY_OPERATIONAL_DUTY (50)			    //< Duty cycle for generated signal at operational mode [%]
#define RELAY_OPERATIONAL_FREQ (10000)			//< Frequency of generated signal at operational mode   [Hz]


/* Variables  ------------------------------------------------------------------------------  */

extern uint32_t pwmStartupStart;


 /* Functions' prototypes ------------------------------------------------------------------   */

/*
	 ==============================================================================
						   ##### PWM API #####
	 ==============================================================================
*/

 /*
  * @brief Initialization function for PWM's PWM module
  * @param[in]   BMS_TypeDef* bms - pointer to handle of BMS struct
  * @param[in]   TIM_HandleTypeDef* htim - pointer to handle of timer
  * @retval[out] HAL_StatusTypeDef - status of operation completion
  */
HAL_StatusTypeDef BMS_PWM_Init(BMS_TypeDef* bms, TIM_HandleTypeDef* htim);

/*
  * @brief Change of status function for PWM's PWM module
  * @param[in]   BMS_TypeDef* bms - pointer to handle of BMS struct
  * @param[in]   PWM_BMSStatusTypeDef - state to be overwritten into BMS's PWM module state
  * @retval[out] HAL_StatusTypeDef - status of operation completion
  */
HAL_StatusTypeDef BMS_PWM_ChandeMode(BMS_TypeDef* bms, PWM_BMSStatusTypeDef status);

/*
  * @brief Operational function for PWM's PWM module
  * @param[in]   BMS_TypeDef* bms - pointer to handle of BMS struct
  * @retval[out] HAL_StatusTypeDef - status of operation completion
  */
HAL_StatusTypeDef BMS_PWM_NormalMode(BMS_TypeDef* bms);

/*
  * @brief Sleep mode function for PWM's PWM module
  * @param[in]   BMS_TypeDef* bms - pointer to handle of BMS struct
  * @retval[out] HAL_StatusTypeDef - status of operation completion
  */
HAL_StatusTypeDef BMS_PWM_SleepMode(BMS_TypeDef* bms);

#ifdef __cplusplus
}
#endif

#endif /* BMS_PWM_H_ */
