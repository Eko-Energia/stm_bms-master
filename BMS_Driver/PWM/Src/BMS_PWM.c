/**
  ******************************************************************************
  * @file      BMS_PWM.c
  * @author    Bartosz Rychlicki
  * @Title     C source files of all functions' bodies required for implementation for BMS Master's PWM module
  * @brief     Functions contain logical implementation of BMS PWM signal generation.
  *
  ******************************************************************************
  * @attention Error codes are called when exact incorrect use of function is made
  *
  * Copyright (c) 2026 AGH Eko-Energy.
  * All rights reserved.
  *
  ******************************************************************************
  */

#include "BMS_PWM.h"

/* Variables ---------------------------------------------------------*/
uint32_t pwmStartupStart;			//< Tick for startup phase

/* Functions' bodies -------------------------------------------------*/

HAL_StatusTypeDef BMS_PWM_Init(BMS_TypeDef* bms, TIM_HandleTypeDef* htim){

	// Checking if correct pointer was given
	if(NULL == bms || NULL == htim){
		return HAL_ERROR;
	}

	// Initializing out signal for PWM generation
	PWM_Out_Init(&bms->bpwm.htim, htim, TIM_CH3, RELAY_STARTUP_DUTY, RELAY_STARTUP_FREQ);

	return HAL_OK;
}

HAL_StatusTypeDef BMS_PWM_ChandeMode(BMS_TypeDef* bms, PWM_BMSStatusTypeDef status){

	// Checking ig correct args were given
	if(NULL == bms || (status != PWM_Startup && status != PWM_Operational)){
		return HAL_ERROR;
	}

	// Change of status
	bms->bpwm.status = status;

	return HAL_OK;
}

HAL_StatusTypeDef BMS_PWM_NormalMode(BMS_TypeDef* bms){

	// Checking if correct pointer was given
	if(NULL == bms){
		return HAL_ERROR;
	}

	// Updating PWM duty cycle: 100% during startup window, then 50% operational
	PWM_Out_setDuty(&bms->bpwm.htim, (bms->bpwm.status == PWM_Startup) ? RELAY_STARTUP_DUTY : RELAY_OPERATIONAL_DUTY);

	// After STARTUP_PERIOD [ms], switch PWM module status to operational
	if(HAL_GetTick() - pwmStartupStart >= STARTUP_PERIOD){

		// Change internal PWM status — duty above follows RELAY_OPERATIONAL_DUTY
		if(BMS_PWM_ChandeMode(bms, PWM_Operational) != HAL_OK){
			return HAL_ERROR;
		}

	}

	return HAL_OK;
}

HAL_StatusTypeDef BMS_PWM_SleepMode(BMS_TypeDef* bms){


	// Checking if correct pointer was given
	if(NULL == bms){
		return HAL_ERROR;
	}

	// Empty implementation - for future low power mode implementation


	return HAL_OK;
}
