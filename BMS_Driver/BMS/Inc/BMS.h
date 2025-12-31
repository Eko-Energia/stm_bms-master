/**
  ******************************************************************************
  * @file    BMS.h
  * @author  Bartosz Rychlicki

  * @Title   Firmware for BMS Master PCB board
  *
  * @brief   This file contains common defines, flags and macros that are used to provide high quality workflow of BMS functionalities.
  *
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 AGH Eko-Energy.
  * All rights reserved.
  *
  ******************************************************************************
  */

#ifndef INC_BMS_H_
#define INC_BMS_H_

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Includes
#include "adc_driver.h"
#include "can_driver.h"
#include "can_id_list.h"
#include "string.h"
#include "BMS_ADC_driver.h"
#include "BMS_CAN_driver.h"
#include "BMS_TIM_driver.h"

/**
  * @brief  BMS's mode status | enumeration type
  */
typedef enum{
	BMS_NORMAL = 0,
	BMS_STANDBY,
	BMS_Error
}BMS_StatusTypeDef_e;


/**
  * @brief  BMS object type
  */
typedef struct{
	ADC_ChannelsTypeDef  cadc1;														// ADC1's Channels' configurations object
	ADC_BufferTypeDef	 badc1;														// ADC1's Channels' converted value buffer

	BMS_StatusTypeDef_e  status;

	uint16_t ADC_voltTempCurr[3];
	uint16_t CAN2_voltageCells[7][9];

	CAN_ScheduledMsgList CAN1_Buff;
}BMS_TypeDef;


// Macros
#define BMS_VCC_SUPPLY      87.0f
#define VCC_SUPPLY_VOLTAGE  3.3f

/* Functions Prototypes --------------------------------------------------------------------  */
HAL_StatusTypeDef BMS_Init(BMS_TypeDef* bms,  CAN_HandleTypeDef* bhcan1, CAN_HandleTypeDef* bhcan2, TIM_HandleTypeDef* htim);

HAL_StatusTypeDef BMS_Mode_Normal(BMS_TypeDef* bms, TIM_HandleTypeDef* htim, ADC_HandleTypeDef* hadc);

HAL_StatusTypeDef BMS_Mode_Standby(BMS_TypeDef* bms, TIM_HandleTypeDef* htim);

HAL_StatusTypeDef BMS_Mode_Error(BMS_TypeDef* bms, TIM_HandleTypeDef* htim);

HAL_StatusTypeDef BMS_Log_Data(BMS_TypeDef* bms);




#endif /* INC_BMS_H_ */
