/**
  ******************************************************************************
  * @file    BMS_ADC_driver.h.h
  * @author  Bartosz Rychlicki

  * @Title   Firmware for BMS Master PCB board's ADCs
  *
  * @brief   This file contains common defines, flags and macros that are used to provide high quality workflow of ADCs embedded in BMS Msster.
  *
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 AGH Eko-Energy.
  * All rights reserved.
  *
  ******************************************************************************
  */
#ifndef INC_BMS_ADC_DRIVER_H_
#define INC_BMS_ADC_DRIVER_H_

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ----------------------------------------------------------------------------------  */
#include "BMS_Types.h"

/* Macros ------------------------------------------------------------------------------------  */



/* Functions' prototypes ---------------------------------------------------------------------  */
HAL_StatusTypeDef BMS_ADC_Init(BMS_TypeDef* bms);

HAL_StatusTypeDef BMS_ADC_ReadValues(BMS_TypeDef* bms);

HAL_StatusTypeDef BMS_ADC_Read_Voltage(BMS_TypeDef* bms);

HAL_StatusTypeDef BMS_ADC_Read_Temperature(BMS_TypeDef* bms);

HAL_StatusTypeDef BMS_ADC_Read_Current(BMS_TypeDef* bms);

float BMS_ADC_NTC_calibrateTemperature(float measured);

float BMS_ADC_NTC_GetTemperature(float Rt);


#ifdef __cplusplus
}
#endif

#endif /* INC_BMS_ADC_DRIVER_H_ */
