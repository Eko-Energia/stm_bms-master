/**
  ******************************************************************************
  * @file    BMS_CAN_driver.h
  * @author  Bartosz Rychlicki

  * @Title   Firmware for CAN peripheral of BMS Master PCB board
  *
  * @brief   This file contains common defines, flags and macros that are used to provide high quality workflow of CAN embedded in BMS.
  *
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 AGH Eko-Energy.
  * All rights reserved.
  *
  ******************************************************************************
  */

#ifndef INC_BMS_CAN_DRIVER_H_
#define INC_BMS_CAN_DRIVER_H_

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* Includes --------------------------------------------------------------------------------  */
#include "BMS_Types.h"

/* Variables ---------------------------------------------------------*/

/* Macros ----------------------------------------------------------------------------------  */
/*
	 ==============================================================================
						   ##### CAN1 TX MACROS #####
	 ==============================================================================
*/

// CAN1 Scaling parameters
#define VOLTAGE_OFFSET          (0.0f)
#define VOLTAGE_GAIN 		    (0.1f)

#define CURRENT_OFFSET          (300.0f)
#define CURRENT_GAIN 		    (0.1f)

#define TEMPERATURE_OFFSET      (0.0f)
#define TEMPERATURE_GAIN        (0.01f)

// CAN2 scalling params
#define THERM_TEMPERATURE_GAIN  (0.39216f)

// CAN Frames DLCs
#define BMS_NODE_DLC		    (8)
#define BMS_VOLTCURTEMP_DLC     (6)
#define BMS_THERMx_DLC		    (7)


//CAN Frames Periods
#define BMS_NODE_PERIOD		    (5000)
#define BMS_VOLTCURTEMP_PERIOD  (500)
#define BMS_THERMx_PERIOD		(1000)

// CAN Frames IDs
#define BMS_NODE_ID		        (128)
#define BMS_VOLTCURTEMP_ID      (130)
#define BMS_THERM1_ID		    (131)
#define BMS_THERM2_ID		    (132)
#define BMS_THERM3_ID			(133)
#define BMS_THERM4_ID			(134)
#define BMS_THERM5_ID			(135)
#define BMS_THERM6_ID			(136)
#define BMS_THERM7_ID			(137)
#define BMS_THERM8_ID			(138)
#define BMS_THERM9_ID			(139)



/* Functions' prototypes ------------------------------------------------------------------  */

HAL_StatusTypeDef BMS_CAN_Init(BMS_TypeDef* bms);

HAL_StatusTypeDef BMS_CAN_AddMessage(BMS_TypeDef* bms, uint32_t Id, uint8_t DLC, uint32_t period);

HAL_StatusTypeDef BMS_CAN_AddPeripheralFrames(BMS_TypeDef* bms);

void 			  BMS_CAN_Get_ADC_Data(uint8_t *data, void *context);

void 			  BMS_CAN_PackCAN2Temps(uint8_t* data, uint8_t thermId);

void 			  BMS_CAN_Get_CAN2_Data_Therm1(uint8_t *data, void* context);

void 			  BMS_CAN_Get_CAN2_Data_Therm2(uint8_t *data, void* context);

void 			  BMS_CAN_Get_CAN2_Data_Therm3(uint8_t *data, void* context);

void 			  BMS_CAN_Get_CAN2_Data_Therm4(uint8_t *data, void* context);

void 			  BMS_CAN_Get_CAN2_Data_Therm5(uint8_t *data, void* context);

void 			  BMS_CAN_Get_CAN2_Data_Therm6(uint8_t *data, void* context);

void 			  BMS_CAN_Get_CAN2_Data_Therm7(uint8_t *data, void* context);

void 			  BMS_CAN_Get_CAN2_Data_Therm8(uint8_t *data, void* context);

void 			  BMS_CAN_Get_CAN2_Data_Therm9(uint8_t *data, void* context);

uint8_t 		  BMS_CAN_GetMSB(uint16_t value);

uint8_t 		  BMS_CAN_GetLSB(uint16_t value);

HAL_StatusTypeDef BMS_CAN_ScallingParams(BMS_TypeDef* bms, uint8_t channel, float* value_f);

HAL_StatusTypeDef HAL_CAN_HandleRxMsg(CAN_HandleTypeDef *hcan);

void 			  HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan);

#ifdef __cplusplus
}
#endif

#endif	/* INC_BMS_CAN_DRIVER_H_ */
