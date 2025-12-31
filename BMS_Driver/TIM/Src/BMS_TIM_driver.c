/**
  ******************************************************************************
  * @file      BMS.c
  * @author    Bartosz Rychlicki
  * @Title     C source files of all functions' bodies required for implementation for BMS Master's TIM peripheral
  * @brief     Functions contain logical implementation of BMS TIM workflow.
  *
  ******************************************************************************
  * @attention Error codes are called when exact incorrect use of function is made
  *
  * Copyright (c) 2025 AGH Eko-Energy.
  * All rights reserved.
  *
  ******************************************************************************
  */

#include "BMS_TIM_driver.h"

HAL_StatusTypeDef BMS_TIM_LED_Blink(BMS_StatusTypeDef_e status, TIM_HandleTypeDef* htim){

	// Universal variables
	uint32_t currentTick = HAL_GetTick();
	uint16_t duty = 0;

	// Standby mode variables
	uint32_t phase = currentTick % 4000; // set standby phase for 2s


	// state machine
	switch(status){
		case BMS_NORMAL:

			// Turning on Green LED | Blink
			if((currentTick % 1000) < 500){ // checking if current tick is in first half of period
				duty = 500;					// if yes, then set duty to 500 and turn on green LED
			}else{
				duty = 0;					// otherwise, turn off green led
			}
			__HAL_TIM_SET_COMPARE(htim, TIM_GREEN_LD, duty);

			// Turning off Red Led
			__HAL_TIM_SET_COMPARE(htim, TIM_RED_LD, 0);

			break;
		case BMS_STANDBY:



			if(phase < 2000){					    // if current tick is in period of 1s then turn on green led

				duty = 500;

			}else if(phase < 3000){				    // if current tick is in range (1s, 1,5s) then map duty from 1s - 1.5s
												    // to brightness 500 -> 0
				duty = 500 - (phase - 2000) / 2;	// duty range: (1, 500). Dividing by 2 is required for correct mapping of duty value

			}else{

				duty = (phase - 3000) / 2;			// mapping brightness to 0 -> 500. Dividing by 2 is required for correct mapping of duty value

			}

			// Turning on Green LED | Duty Cycle = 75%
			__HAL_TIM_SET_COMPARE(htim, TIM_GREEN_LD, duty);

			// Turning off Red Led
			__HAL_TIM_SET_COMPARE(htim, TIM_RED_LD, 0);

			break;
		case BMS_Error:

			// Turning off Green LED
			__HAL_TIM_SET_COMPARE(htim, TIM_GREEN_LD, 0);

			// Turning on Red Led
			__HAL_TIM_SET_COMPARE(htim, TIM_RED_LD, 999);

			break;
		default:

			// returning error in case of wrong state
			return HAL_ERROR;

			break;
	}

	return HAL_OK;
}
