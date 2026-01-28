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

/* Includes -----------------------------------------------------------------------------------  */
#include "BMS.h"

/* Variables ----------------------------------------------------------------------------------  */
extern uint32_t lastTick;

/* Functions' bodies --------------------------------------------------------------------------  */
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



	// Read ADC's channels
	if(BMS_ADC_ReadValues(bms, hadc) != HAL_OK){
		Error_Handler();
	}

	// Update CAN Messages list

	// Send Data via CAN

	// Send Data via nrf905


	return HAL_OK;
}

HAL_StatusTypeDef BMS_Mode_Error(BMS_TypeDef* bms, TIM_HandleTypeDef* htim){




	return HAL_OK;
}

HAL_StatusTypeDef BMS_Log_Data(BMS_TypeDef* bms){

	return HAL_OK;
}

void BMS_Mode_LEDBlink(BMS_TypeDef* bms){

	// init variable which stores current tick
	uint32_t now = HAL_GetTick();

	// checking if correct ammout of time passed to Toggle LED state
	if(now - lastTick >= 500){

		switch(bms->status){

			// toggling green state in case Normal mode is running
			case BMS_NORMAL:

				HAL_GPIO_TogglePin(GREEN_LD_GPIO_Port, GREEN_LD_Pin);
				break;

			// toggling red LED state in case Error mode is running
			case BMS_Error:

				HAL_GPIO_TogglePin(RED_LD_GPIO_Port, RED_LD_Pin);
				break;
		}

		// updating tick
		lastTick = now;

	}

}

