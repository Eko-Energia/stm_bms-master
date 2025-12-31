/**
  ******************************************************************************
  * @file      BMS.c
  * @author    Bartosz Rychlicki
  * @Title     C source files of all functions' bodies required for implementation for BMS Master's PCB board
  * @brief     Functions contain logical implementation of BMS workflow.
  *
  ******************************************************************************
  * @attention Error codes are called when exact incorrect use of function is made
  *
  * Copyright (c) 2025 AGH Eko-Energy.
  * All rights reserved.
  *
  ******************************************************************************
  */
#include "BMS.h"

HAL_StatusTypeDef BMS_Init(BMS_TypeDef* bms,  CAN_HandleTypeDef* bhcan1, CAN_HandleTypeDef* bhcan2, TIM_HandleTypeDef* htim){

	// setting default status (normal) for BMS
	bms->status = BMS_NORMAL;

	// reseting array which stores voltage, temperature and current, whose values are converted by ADC
	memset(bms->ADC_voltTempCurr, 0 , sizeof(bms->ADC_voltTempCurr));

	// reseting array which stores cells' voltages, whose values are provided by CAN2
	for(int i = 0; i< 7; ++i){ memset(bms->CAN2_voltageCells[i], 0, sizeof(bms->CAN2_voltageCells[i][0]));}

	// Init CAN for bms
	if(BMS_CAN_Init(bms, bhcan1, bhcan2) != HAL_OK){
		return HAL_ERROR;
	}

	// Starting timers on channels 4 for handling blinking green LEDs
	if(HAL_TIM_PWM_Start(htim, TIM_GREEN_LD) != HAL_OK){
		return HAL_ERROR;
    }

	// Starting timer on channels 3 for handling blinking red LEDs
	if(HAL_TIM_PWM_Start(htim, TIM_RED_LD) != HAL_OK){
		return HAL_ERROR;
  	}

	return HAL_OK;
}

HAL_StatusTypeDef BMS_Mode_Normal(BMS_TypeDef* bms, TIM_HandleTypeDef* htim, ADC_HandleTypeDef* hadc){

	// blink green LED PWM
	if(BMS_TIM_LED_Blink(bms->status, htim) != HAL_OK){
		Error_Handler();
	}


	// Read ADC's channels
	if(BMS_ADC_ReadValues(bms, hadc) != HAL_OK){
		Error_Handler();
	}

	// Update CAN Messages list

	// Send Data via CAN

	// Send Data via nrf905


	return HAL_OK;
}

HAL_StatusTypeDef BMS_Mode_Standby(BMS_TypeDef* bms, TIM_HandleTypeDef* htim){

	// blink green LED with PWM
	if(BMS_TIM_LED_Blink(bms->status, htim) != HAL_OK){
		Error_Handler();
	}

	return HAL_OK;
}

HAL_StatusTypeDef BMS_Mode_Error(BMS_TypeDef* bms, TIM_HandleTypeDef* htim){


	// turning on red LED with PWM
	if(BMS_TIM_LED_Blink(bms->status, htim) != HAL_OK){
		Error_Handler();
	}

	return HAL_OK;
}

HAL_StatusTypeDef BMS_Log_Data(BMS_TypeDef* bms){

	return HAL_OK;
}

