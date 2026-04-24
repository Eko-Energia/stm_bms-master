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

/* Variables ---------------------------------------------------------*/
extern BMS_TypeDef bms;


/* Functions' bodies -------------------------------------------------*/
HAL_StatusTypeDef BMS_CAN_Init(BMS_TypeDef* bms){

	// Init of CAN1 and CAN2 to start communication via these buses
	CAN_Init(bms->bmsCAN.bhcan1);
	CAN_Init(bms->bmsCAN.bhcan2);

	// launching interrupts for CAN2
	if(HAL_CAN_ActivateNotification(bms->bmsCAN.bhcan2, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK){
		return HAL_ERROR;
	}

	// Launching CAN1
	CAN_Init(bms->bmsCAN.bhcan1);

	// Adding frames co-related to peripherals data
	if(BMS_CAN_AddPeripheralFrames(bms) != HAL_OK){
		return HAL_ERROR;
	}

	// Launching RCC clock for CAN1 and CAN2
	__HAL_RCC_CAN1_CLK_ENABLE();
	__HAL_RCC_CAN2_CLK_ENABLE();

	return HAL_OK;
}

HAL_StatusTypeDef BMS_CAN_AddMessage(BMS_TypeDef* bms, uint32_t Id, uint8_t DLC, uint32_t period){

	// initialize CAN message
	struct CAN_scheduledMsg msg;

	// basic setup of CAN frame
	msg.header.IDE = CAN_ID_STD;
	msg.header.RTR = CAN_RTR_DATA;
	msg.header.StdId = Id;
	msg.header.DLC = DLC;
	msg.periodMs = period;
	msg.header.ExtId = 0;

	// assigning correct return of data function to correct msg
	switch(Id){
		case BMS_VOLTCURTEMP_ID:
			msg.getData = BMS_CAN_Get_ADC_Data;
			break;
		case BMS_THERM1_ID:
			msg.getData = BMS_CAN_Get_CAN2_Data_Therm1;
			break;
		case BMS_THERM2_ID:
			msg.getData = BMS_CAN_Get_CAN2_Data_Therm2;
				break;
		case BMS_THERM3_ID:
			msg.getData = BMS_CAN_Get_CAN2_Data_Therm3;
				break;
		case BMS_THERM4_ID:
			msg.getData = BMS_CAN_Get_CAN2_Data_Therm4;
				break;
		case BMS_THERM5_ID:
			msg.getData = BMS_CAN_Get_CAN2_Data_Therm5;
				break;
		case BMS_THERM6_ID:
			msg.getData = BMS_CAN_Get_CAN2_Data_Therm6;
				break;
		case BMS_THERM7_ID:
			msg.getData = BMS_CAN_Get_CAN2_Data_Therm7;
				break;
		case BMS_THERM8_ID:
			msg.getData = BMS_CAN_Get_CAN2_Data_Therm8;
				break;
		case BMS_THERM9_ID:
			msg.getData = BMS_CAN_Get_CAN2_Data_Therm9;
				break;
		default:

			// return error status in case wrong Id has been given
			return HAL_ERROR;
		break;
	}


	// adding frame to CAN frames' buffer
	if(CAN_AddScheduledMsg(&msg, &bms->bmsCAN.CAN1_Buff) != HAL_OK){
		return  HAL_ERROR;
	}

	return HAL_OK;
}

HAL_StatusTypeDef BMS_CAN_AddPeripheralFrames(BMS_TypeDef* bms){

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

void BMS_CAN_Get_ADC_Data(uint8_t *data, void* context){

	// setting data with ADC's measured voltage
	data[0] = BMS_CAN_GetLSB(bms.bmsADC.ADC_voltTempCurr[0]);	// LSB
	data[1] = BMS_CAN_GetMSB(bms.bmsADC.ADC_voltTempCurr[0]);   // MSB

	// setting data with ADC's measured current
	data[2] = BMS_CAN_GetLSB(bms.bmsADC.ADC_voltTempCurr[2]);	// LSB
	data[3] = BMS_CAN_GetMSB(bms.bmsADC.ADC_voltTempCurr[2]);   // MSB

	// setting data with ADC's measured temperature
	data[4] = BMS_CAN_GetLSB(bms.bmsADC.ADC_voltTempCurr[1]);	// LSB
	data[5] = BMS_CAN_GetMSB(bms.bmsADC.ADC_voltTempCurr[1]);   // MSB
}

void BMS_CAN_PackCAN2Temps(uint8_t* data, uint8_t thermId){
	for(int i = 0; i < 7; ++i){
		data[i] = (uint8_t)bms.bmsCAN.CAN2_temperatureCells[i][thermId];
	}
}


void BMS_CAN_Get_CAN2_Data_Therm1(uint8_t *data, void* context){ BMS_CAN_PackCAN2Temps(data, 0);}
void BMS_CAN_Get_CAN2_Data_Therm2(uint8_t *data, void* context){ BMS_CAN_PackCAN2Temps(data, 1);}
void BMS_CAN_Get_CAN2_Data_Therm3(uint8_t *data, void* context){ BMS_CAN_PackCAN2Temps(data, 2);}
void BMS_CAN_Get_CAN2_Data_Therm4(uint8_t *data, void* context){ BMS_CAN_PackCAN2Temps(data, 3);}
void BMS_CAN_Get_CAN2_Data_Therm5(uint8_t *data, void* context){ BMS_CAN_PackCAN2Temps(data, 4);}
void BMS_CAN_Get_CAN2_Data_Therm6(uint8_t *data, void* context){ BMS_CAN_PackCAN2Temps(data, 5);}
void BMS_CAN_Get_CAN2_Data_Therm7(uint8_t *data, void* context){ BMS_CAN_PackCAN2Temps(data, 6);}
void BMS_CAN_Get_CAN2_Data_Therm8(uint8_t *data, void* context){ BMS_CAN_PackCAN2Temps(data, 7);}
void BMS_CAN_Get_CAN2_Data_Therm9(uint8_t *data, void* context){ BMS_CAN_PackCAN2Temps(data, 8);}


uint8_t BMS_CAN_GetMSB(uint16_t value){
	return (uint8_t)(value >> 8);
}

uint8_t BMS_CAN_GetLSB(uint16_t value){
	return (uint8_t)value;
}


HAL_StatusTypeDef BMS_CAN_ScallingParams(BMS_TypeDef* bms, uint8_t channel, float* value_f){

	switch(channel){
		case ADC_VOLTAGE_CH:

			// calculating binary type of read voltage with factor and offset
			bms->bmsADC.ADC_voltTempCurr[0] = (*value_f - VOLTAGE_OFFSET)     / VOLTAGE_GAIN;
			break;
		case ADC_CURRENT_CH:

			// calculating binary type of read voltage with factor and offset
			bms->bmsADC.ADC_voltTempCurr[2] = (*value_f - CURRENT_OFFSET)     / CURRENT_GAIN;
			break;
		case ADC_TEMP_CH:

			// calculating binary type of read voltage with factor and offset
			bms->bmsADC.ADC_voltTempCurr[1] = (*value_f - TEMPERATURE_OFFSET) / TEMPERATURE_GAIN;
			break;
		default:

			// returning error state in case incorrect channel was given
			return HAL_ERROR;
			break;
	}

	return HAL_OK;
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan){
	if(hcan->Instance == CAN2){

		static CAN_RxHeaderTypeDef RxHeader;	// Init header for Rx frame
		static uint8_t* rxData;				    // Init data storage for Rx frame's data

		float  thermTemperatyure = 0.0f;       // temperature to eventually trigger EH if its value it too high

		if(HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, rxData) == HAL_OK){

			// calculating received Number of PCB which sent frame and thermistor index, whose temperature has been sent.
			int pcbIndex = (RxHeader.StdId - 200) / 10;						// extracting number from Id, PCB index stand as a second number in frame's ID
			uint8_t thermIndex = RxHeader.StdId - 200 - pcbIndex * 10;		// extracting number from Id, thermistor index stand as a third number in frame's ID

			thermTemperatyure = (float)*rxData * THERM_TEMPERATURE_GAIN;

			if(thermTemperatyure >= TEMP_MAX){
				// Report to EH
				EH_report(&bms.beh, EH_CAN2_TEMP_HIGH, ERROR_SEVERITY_SAFE_STATE);

			}


			// overwriting container for cells' temperatures with new value. indexes are decreamented cause indexes in arrays starts from index 0, but calculated numbers start from 1
			bms.bmsCAN.CAN2_temperatureCells[pcbIndex - 1][thermIndex - 1] = rxData[0];

		}
	}
}
