/**
  ******************************************************************************
  * @file      BMS.c
  * @author    Bartosz Rychlicki
  * @Title     C source files of all functions' bodies required for implementation for BMS Master's CAN peripheral
  * @brief     Functions contain logical implementation of BMS CAN workflow.
  *
  ******************************************************************************
  * @attention Error codes are called when exact incorrect use of function is made
  *
  * Copyright (c) 2025 AGH Eko-Energy.
  * All rights reserved.
  *
  ******************************************************************************
  */

#include "BMS_CAN_driver.h"


HAL_StatusTypeDef BMS_CAN_Init(BMS_TypeDef* bms, CAN_HandleTypeDef* bhcan1, CAN_HandleTypeDef* bhcan2){

	// Init of CAN1 and CAN2 to start communication via these buses
	CAN_Init(bhcan1);
	CAN_Init(bhcan2);


	/*
	 * adding all CAN frames, which are co-related to BMS
	 */

	// adding node's frames
	if(BMS_CAN_AddMessage(bms, BMS_NODE_ID, BMS_NODE_DLC, BMS_NODE_PERIOD) != HAL_OK){
		return HAL_ERROR;
	}

	// adding frames with voltage, temperature and current
	if(BMS_CAN_AddMessage(bms, BMS_VOLTCURTEMP_ID, BMS_VOLTCURTEMP_DLC, BMS_VOLTCURTEMP_PERIOD) != HAL_OK){
		return HAL_ERROR;
	}

	// Adding frames co-related to PCBs' cells' temperatures of thermistors
	for(int i = 0; i < 9; ++i){
		if(BMS_CAN_AddMessage(bms, (BMS_THERM1_ID + i), BMS_THERMx_DLC, BMS_THERMx_PERIOD) != HAL_OK){
			return HAL_ERROR;
		}
	}

	return HAL_OK;
}

HAL_StatusTypeDef BMS_CAN_AddMessage(BMS_TypeDef* bms, uint32_t Id, uint8_t DLC, uint32_t period){

	// initialize CAN message
	CAN_ScheduledMsg msg;

	// basic setup of CAN frame
	msg.header.IDE = CAN_ID_STD;
	msg.header.RTR = CAN_RTR_DATA;
	msg.header.StdId = Id;
	msg.header.DLC = DLC;
	msg.period_ms = period;
	msg.header.ExtId = 0;

	// adding frame to CAN frames' buffer
	if(CAN_AddScheduledMessage(msg, &bms->CAN1_Buff) != HAL_OK){
		return  HAL_ERROR;
	}

	return HAL_OK;
}

HAL_StatusTypeDef BMS_CAN_UpdateMsg(BMS_TypeDef* bms){

	// iterating though CAN buffer to update all frames
	for(int i = 0; i < bms->CAN1_Buff.size; ++i){

		if(bms->CAN1_Buff.list[i].header.StdId == BMS_NODE_ID){ 		     // Updating frames co-related Safe state, severity and error codes




		}else if(bms->CAN1_Buff.list[i].header.StdId == BMS_VOLTCURTEMP_ID){ // Updating frames co-related to ADC converted values



		}else{																 // Updating frames co-related to thermistors's measured temperatures from PCB cells



		}
	}


	return HAL_OK;
}


uint8_t BMS_CAN_GetMSB(uint16_t value){
	return (uint8_t)(value >> 8);
}

uint8_t BMS_CAN_GetLSB(uint16_t value){
	return (uint8_t)value;
}



HAL_StatusTypeDef BMS_CAN_ScallingParams(BMS_TypeDef* bms, uint8_t channel, float value_f){

	switch(channel){
		case ADC_VOLTAGE_CH:

			// calculating binary type of read voltage with factor and offset
			bms->ADC_voltTempCurr[0] = (value_f - VOLTAGE_OFFSET) * VOLTAGE_GAIN;
			break;
		case ADC_CURRENT_CH:

			// calculating binary type of read voltage with factor and offset
			bms->ADC_voltTempCurr[2] = (value_f - CURRENT_OFFSET) * CURRENT_GAIN;
			break;
		case ADC_TEMP_CH:

			// calculating binary type of read voltage with factor and offset
			bms->ADC_voltTempCurr[1] = (value_f - TEMPERATURE_OFFSET) * TEMPERATURE_GAIN;
			break;
		default:

			// returning error state in case incorrect channel was given
			return HAL_ERROR;
			break;
	}

	return HAL_OK;
}
