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

HAL_StatusTypeDef BMS_Init(BMS_TypeDef* bms,  CAN_HandleTypeDef* bhcan1, CAN_HandleTypeDef* bhcan2, TIM_HandleTypeDef* htim, ADC_HandleTypeDef* hadc, SPI_HandleTypeDef* hspi, UART_HandleTypeDef* huart){

	// assigning handle objects


	// setting default status (normal) for BMS
	bms->status     = BMS_NORMAL;
	bms->prevStatus = BMS_NORMAL;

	// reseting array which stores voltage, temperature and current, whose values are converted by ADC
	memset(bms->bmsADC.ADC_voltTempCurr, 0 , sizeof(bms->bmsADC.ADC_voltTempCurr));

	// reseting array which stores cells' voltages, whose values are provided by CAN2
	for(int i = 0; i< 7; ++i){ memset(bms->bmsCAN.CAN2_temperatureCells[i], 0, sizeof(bms->bmsCAN.CAN2_temperatureCells[i][0]));}


	// Starting timers on channels 4 for handling blinking green LEDs
	if(HAL_TIM_PWM_Start(&bms->htim, TIM_GREEN_LD) != HAL_OK){
		return HAL_ERROR;
    }


	// Starting timer on channels 3 for handling blinking red LEDs
	if(HAL_TIM_PWM_Start(&bms->htim, TIM_RED_LD) != HAL_OK){
		return HAL_ERROR;
  	}

	if(BMS_Start_Peripherals(bms) != HAL_OK){
		return HAL_ERROR;
	}


	return HAL_OK;
}

HAL_StatusTypeDef BMS_Mode_Normal(BMS_TypeDef* bms){

	// re-launching peripherals in case change of status occurred
	if(bms->prevStatus != BMS_NORMAL){

		if(BMS_Start_Peripherals(bms) != HAL_OK){
			return HAL_ERROR;
		}

	}

	// Read ADC's channels
	if(BMS_ADC_ReadValues(bms) != HAL_OK){
		Error_Handler();
	}

	// Send Data via CAN
	CAN_HandleScheduled(&bms->bmsCAN.bhcan1, &bms->bmsCAN.CAN1_Buff);


	// Send Data via nrf905


	return HAL_OK;
}

HAL_StatusTypeDef BMS_Mode_Standby(BMS_TypeDef* bms){



	return HAL_OK;
}

HAL_StatusTypeDef BMS_Mode_Error(BMS_TypeDef* bms){




	return HAL_OK;
}

HAL_StatusTypeDef BMS_Log_Data(BMS_TypeDef* bms){


	return HAL_OK;
}

HAL_StatusTypeDef BMS_Start_Peripherals(BMS_TypeDef* bms){

/*
	 ==============================================================================
	                       ##### LAUNCHING ADC #####
	 ==============================================================================
*/
	// Launching DMA for ADC
	if(HAL_ADC_Start_DMA(&bms->bmsADC.hadc, (uint32_t*)bms->bmsADC.badc1.idma.BufferADC, ADC_BUFF_SIZE) != HAL_OK){
		return HAL_ERROR;
	}

	// Launching ADC for BMS
	if(ADC_Init(&bms->bmsADC.hadc, &bms->bmsADC.badc1, &bms->bmsADC.cadc1) != HAL_OK){
		return HAL_OK;
	}

/*
	 ==============================================================================
						   ##### LAUNCHING CAN #####
	 ==============================================================================
*/

	// Launching CAN for BMS
	if(BMS_CAN_Init(bms) != HAL_OK){
		return HAL_ERROR;
	}

	// Checking if CAN1 is in sleep mode, if yes, then wake up CAN1
	if(HAL_CAN_IsSleepActive(&bms->bmsCAN.bhcan1)){
		if(HAL_CAN_WakeUp(&bms->bmsCAN.bhcan1) != HAL_OK){
			return HAL_ERROR;
		}
	}

	// Checking if CAN2 is in sleep mode, if yes, then wake up CAN2
	if(HAL_CAN_IsSleepActive(&bms->bmsCAN.bhcan2)){
		if(HAL_CAN_WakeUp(&bms->bmsCAN.bhcan2) != HAL_OK){
			return HAL_ERROR;
		}
	}

	return HAL_OK;
}


HAL_StatusTypeDef BMS_Stop_Peripherals(BMS_TypeDef* bms){

/*
	 ==============================================================================
						   ##### STOPPING ADC #####
	 ==============================================================================
*/

	// Stopping ADC peripheral workflow for BMS in Standby or Error Mode
	if(HAL_ADC_Stop(&bms->bmsADC.hadc) != HAL_OK){
		return HAL_ERROR;
	}

	if(HAL_ADC_Stop_DMA(&bms->bmsADC.hadc) !=  HAL_OK){
		return HAL_OK;
	}

/*
	 ==============================================================================
						   ##### STOPPING CAN #####
	 ==============================================================================
*/

	// Stopping CAN1 peripheral workflow for BMS in Standby or Error Mode
	if(HAL_CAN_Stop(&bms->bmsCAN.bhcan1) != HAL_OK){
		return HAL_ERROR;
	}

	// Stopping CAN2 peripheral workflow for BMS in Standby or Error Mode
	if(HAL_CAN_Stop(&bms->bmsCAN.bhcan2) != HAL_OK){
		return HAL_ERROR;
	}

	// Deactivating Interrupts for CAN2
	if(HAL_CAN_DeactivateNotification(&bms->bmsCAN.bhcan2, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK){
		return HAL_ERROR;
	}


	return HAL_OK;
}
