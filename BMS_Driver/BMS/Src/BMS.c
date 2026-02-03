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

/* Includes --------------------------------------------------------------------------------  */
#include "BMS.h"

/* Variables -------------------------------------------------------------------------------  */
extern uint32_t lastTick;

/* Functions' bodies -----------------------------------------------------------------------  */

HAL_StatusTypeDef BMS_Init(BMS_TypeDef* bms,  CAN_HandleTypeDef* bhcan1, CAN_HandleTypeDef* bhcan2, ADC_HandleTypeDef* hadc, SPI_HandleTypeDef* hspi, UART_HandleTypeDef* huart){

	// assigning handle objects
	bms->bmsADC.hadc   = hadc;
	bms->bmsCAN.bhcan1 = bhcan1;
	bms->bmsCAN.bhcan2 = bhcan2;
	bms->hspi1 		   = hspi;
	bms->huart1        = huart;

	// setting default status (normal) for BMS
	bms->status     = BMS_NORMAL;
	bms->prevStatus = BMS_NORMAL;

	// reseting array which stores voltage, temperature and current, whose values are converted by ADC
	memset(bms->bmsADC.ADC_voltTempCurr, 0 , sizeof(bms->bmsADC.ADC_voltTempCurr));

	// reseting array which stores cells' voltages, whose values are provided by CAN2
	for(int i = 0; i< 7; ++i){ memset(bms->bmsCAN.CAN2_temperatureCells[i], 0, sizeof(bms->bmsCAN.CAN2_temperatureCells[i][0]));}

	// Launching CAN1 and CAN2
	if(BMS_CAN_Init(bms) != HAL_OK){
		return HAL_ERROR;
	}

	// Init ADC
	if(BMS_ADC_Init(bms) != HAL_OK){
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

		bms->prevStatus = BMS_NORMAL;
	}

	// Read ADC's channels
	if(BMS_ADC_ReadValues(bms) != HAL_OK){
		return HAL_ERROR;
	}

	// Send Data via CAN
	CAN_HandleScheduled(bms->bmsCAN.bhcan1, &bms->bmsCAN.CAN1_Buff);


	// Send Data via nrf905


	return HAL_OK;
}

HAL_StatusTypeDef BMS_Mode_Error(BMS_TypeDef* bms){




	return HAL_OK;
}

void BMS_Mode_Change(BMS_TypeDef* bms, BMS_StatusTypeDef_e status){


	// saving previous status
	bms->prevStatus = bms->status;

	// overwriting current BMS's status
	bms->status = status;

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
	if(HAL_ADC_Start_DMA(bms->bmsADC.hadc, (uint32_t*)bms->bmsADC.badc1.idma.BufferADC, ADC_BUFF_SIZE) != HAL_OK){
		return HAL_ERROR;
	}


/*
	 ==============================================================================
						   ##### LAUNCHING CAN #####
	 ==============================================================================
*/


	// Checking if CAN2 is in sleep mode, if yes, then wake up CAN2
	if(HAL_CAN_IsSleepActive(bms->bmsCAN.bhcan2)){
		if(HAL_CAN_WakeUp(bms->bmsCAN.bhcan2) != HAL_OK){
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
	if(HAL_ADC_Stop(bms->bmsADC.hadc) != HAL_OK){
		return HAL_ERROR;
	}

	if(HAL_ADC_Stop_DMA(bms->bmsADC.hadc) !=  HAL_OK){
		return HAL_OK;
	}

/*
	 ==============================================================================
						   ##### STOPPING CAN #####
	 ==============================================================================
*/


	// Stopping CAN2 peripheral workflow for BMS in Standby or Error Mode
	if(HAL_CAN_Stop(bms->bmsCAN.bhcan2) != HAL_OK){
		return HAL_ERROR;
	}

	// Deactivating Interrupts for CAN2
	if(HAL_CAN_DeactivateNotification(bms->bmsCAN.bhcan2, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK){
		return HAL_ERROR;
	}


	return HAL_OK;
}

void BMS_Mode_LEDBlink(BMS_TypeDef* bms){


	// init variable which stores current tick
	uint32_t now = HAL_GetTick();

	// checking if correct ammout of time passed to Toggle LED state

	if(now - lastTick >= 500){

		switch(bms->status){

			// executing blinking for normal BMS's state
			case BMS_NORMAL:
				// Toggling GREEN LED
				HAL_GPIO_TogglePin(GREEN_LD_GPIO_Port, GREEN_LD_Pin);

				// Turning off RED lED
				HAL_GPIO_WritePin(RED_LD_GPIO_Port, RED_LD_Pin, GPIO_PIN_RESET);

				break;

			// executing blinking for error BMS's state
			case BMS_Error:

				// Toggling RED LED
				HAL_GPIO_TogglePin(RED_LD_GPIO_Port, RED_LD_Pin);

				// Turning off GREEN lED
				HAL_GPIO_WritePin(GREEN_LD_GPIO_Port, GREEN_LD_Pin, GPIO_PIN_RESET);

				break;
		}

		// updating tick
		lastTick = now;

	}

}
