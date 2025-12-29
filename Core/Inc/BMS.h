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

#include "adc_driver.h"
#include "can_driver.h"
#include "can_id_list.h"
#include "string.h"

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

// ADC Channels
#define ADC_VOLTAGE_CH 12
#define ADC_CURRENT_CH 11
#define ADC_TEMP_CH    10

// CAN Scaling parameters
#define VOLTAGE_OFFSET      12.0f
#define VOLTAGE_GAIN 		0.0059f
#define CURRENT_OFFSET      300.0f
#define CURRENT_GAIN 		0.1465f
#define TEMPERATURE_OFFSET  50.0f
#define TEMPERATURE_GAIN    0.024f

// STM32
#define VCC_SUPPLY_VOLTAGE  3.3f

// TIM
#define TIM_GREEN_LD		TIM_CHANNEL_4
#define TIM_RED_LD   		TIM_CHANNEL_3


/* Functions Prototypes --------------------------------------------------------------------  */
HAL_StatusTypeDef BMS_Init(BMS_TypeDef* bms);

HAL_StatusTypeDef BMS_Mode_Normal(BMS_TypeDef* bms, TIM_HandleTypeDef* htim, ADC_HandleTypeDef* hadc);

HAL_StatusTypeDef BMS_Mode_Standby(BMS_TypeDef* bms, TIM_HandleTypeDef* htim);

HAL_StatusTypeDef BMS_Mode_Error(BMS_TypeDef* bms, TIM_HandleTypeDef* htim);

HAL_StatusTypeDef BMS_Log_Data(BMS_TypeDef* bms);

HAL_StatusTypeDef BMS_CAN_UpdateMsg(BMS_TypeDef* bms);

HAL_StatusTypeDef BMS_ADC_ReadValues(BMS_TypeDef* bms, ADC_HandleTypeDef* hadc);

HAL_StatusTypeDef BMS_ADC_Read_Voltage(BMS_TypeDef* bms, ADC_HandleTypeDef* hadc);

HAL_StatusTypeDef BMS_ADC_Read_Temperature(BMS_TypeDef* bms, ADC_HandleTypeDef* hadc);

HAL_StatusTypeDef BMS_ADC_Read_Current(BMS_TypeDef* bms, ADC_HandleTypeDef* hadc);

HAL_StatusTypeDef BMS_CAN_ScallingParams(BMS_TypeDef* bms, uint8_t channel, float value_f);

float BMS_ADC_NTC_calibrateTemperature(float measured);

float BMS_ADC_NTC_GetTemperature(float Rt);

HAL_StatusTypeDef BMS_LED_Blink(BMS_StatusTypeDef_e status, TIM_HandleTypeDef* htim);


#endif /* INC_BMS_H_ */
