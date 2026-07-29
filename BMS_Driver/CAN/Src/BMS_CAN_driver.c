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
  * Copyright (c) 2026 AGH Eko-Energy.
  * All rights reserved.
  *
  ******************************************************************************
  */

#include "BMS_CAN_driver.h"

/* Variables ---------------------------------------------------------*/
extern BMS_TypeDef 		   bms;								//< Init BMS object
static volatile uint8_t    rxMsgReceived;					//< Declaration of flag that indicates status of received CAN2 frame

static CAN_RxHeaderTypeDef RxHeader = {0};				    //< Init header for Rx frame
static uint8_t 			   rxData[8] = {0};				    //< Init data storage for Rx frame's data


/* Functions' bodies -------------------------------------------------*/
HAL_StatusTypeDef BMS_CAN_Init(BMS_TypeDef* bms){

	// Setting normal state for both transceivers
	HAL_GPIO_WritePin(nCAN1_Stby_GPIO_Port, nCAN1_Stby_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(nCAN2_Stby_GPIO_Port, nCAN2_Stby_Pin, GPIO_PIN_RESET);

	// Init of CAN1 and CAN2 to start communication via these buses
	CAN_Init(bms->bmsCAN.bhcan1);
	CAN_Init(bms->bmsCAN.bhcan2);

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
			bms->bmsADC.ADC_voltTempCurr[0] = (*value_f + VOLTAGE_OFFSET)     / VOLTAGE_GAIN;
			break;
		case ADC_CURRENT_CH:

			// calculating binary type of read voltage with factor and offset
			bms->bmsADC.ADC_voltTempCurr[2] = (*value_f + CURRENT_OFFSET)     / CURRENT_GAIN;
			break;
		case ADC_TEMP_CH:

			// calculating binary type of read voltage with factor and offset
			bms->bmsADC.ADC_voltTempCurr[1] = (*value_f + TEMPERATURE_OFFSET) / TEMPERATURE_GAIN;
			break;
		default:

			// returning error state in case incorrect channel was given
			return HAL_ERROR;
			break;
	}

	return HAL_OK;
}

HAL_StatusTypeDef BMS_CAN_HandleRxMsg(BMS_TypeDef *bms){

	/*
	 * FAN scan state (static): tracks unique thermistors across unordered CAN2 RX.
	 * thermSeen[][]     - flags which (pcb,therm) pairs arrived in the current scan
	 * thermUniqueCount  - number of unique sensors seen so far (target: BMS_THERM_TOTAL)
	 * scanMax           - running maximum temperature within the open scan window
	 */
	static uint8_t thermSeen[BMS_THERM_PCB_COUNT][BMS_THERM_PER_PCB] = {0};
	static uint8_t thermUniqueCount = 0;
	static float   scanMax = -1000.0f;

	uint32_t stdId;
	uint8_t  rxByte;

	/* No new CAN2 frame pending from ISR callback */
	if(1 != rxMsgReceived){
		return HAL_OK;
	}

	/* Consume the RX-pending flag (set in HAL_CAN_RxFifo0MsgPendingCallback) */
	rxMsgReceived = 0;

	/*
	 * Critical section: copy ISR-shared RxHeader/rxData, then unlock.
	 * All further processing runs with interrupts enabled.
	 */
	__disable_irq();

	stdId  = RxHeader.StdId;
	rxByte = rxData[0];

	__enable_irq();

	/*
	 * SAFE_STATE frame (StdId = 1): store status only — do NOT index therm buffer.
	 * Early return prevents out-of-bounds write from (1-200)/10 math.
	 */
	if(SAFE_STATE_ID == stdId){

		bms->safeStateStatus = rxByte;

		return HAL_OK;
	}

	/*
	 * Decode thermistor frame ID:
	 *   StdId = BMS_THERM_ID_BASE + pcb*10 + therm
	 *   pcb ∈ [1..7], therm ∈ [1..9]
	 */
	int pcbIndex   = ((int)stdId - BMS_THERM_ID_BASE) / 10;
	int thermIndex = (int)stdId - BMS_THERM_ID_BASE - pcbIndex * 10;

	/* Reject IDs outside the valid 7x9 thermistor map (HW filter may still pass 0x200..0x27F) */
	if(pcbIndex < 1 || pcbIndex > BMS_THERM_PCB_COUNT ||

		thermIndex < 1 || thermIndex > BMS_THERM_PER_PCB){

		return HAL_OK;
	}

	/* Convert raw payload byte to temperature [°C] */
	float thermTemperature = (float)rxByte * THERM_TEMPERATURE_GAIN;

#ifdef PROD
	/* Production path: escalate over-temperature via error handler (fault policy TBD later) */
	if(thermTemperature >= TEMP_MAX){
		EH_report(&bms->beh, EH_CAN2_TEMP_HIGH, ERROR_SEVERITY_SAFE_STATE);
		return HAL_ERROR;
	}
#endif

	/* Store latest raw byte for this (pcb, therm) — used by CAN1 therm group TX */
	bms->bmsCAN.CAN2_temperatureCells[pcbIndex - 1][thermIndex - 1] = rxByte;

	/* Update running max for the current scan (duplicates may raise scanMax) */
	if(thermTemperature > scanMax){
		scanMax = thermTemperature;
	}

	/* Count each (pcb,therm) only once per scan — arrival order does not matter */
	if(0 == thermSeen[pcbIndex - 1][thermIndex - 1]){

		thermSeen[pcbIndex - 1][thermIndex - 1] = 1;

		thermUniqueCount++;
	}

	/*
	 * Scan complete when BMS_THERM_TOTAL unique thermistors have been seen at least once.
	 * Latch scanMax into maxTemperature for BMS_FAN_Control hysteresis, then reset scan.
	 */
	if(thermUniqueCount >= BMS_THERM_TOTAL){

		bms->maxTemperature = scanMax;					/* latched value used by FAN ON/OFF */

		memset(thermSeen, 0, sizeof(thermSeen));		/* start next scan window */

		thermUniqueCount = 0;
		scanMax = -1000.0f;
	}

	return HAL_OK;
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan){

	if(hcan->Instance == CAN2){

		if(HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, rxData) == HAL_OK){

			// setting flag to proceed received frame
			rxMsgReceived = 1;

		}
	}
}
