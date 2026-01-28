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

/* Includes --------------------------------------------------------------------------------  */
#include "BMS_Types.h"

/* Macros ----------------------------------------------------------------------------------  */


/* Functions Prototypes --------------------------------------------------------------------  */
HAL_StatusTypeDef BMS_Init(BMS_TypeDef* bms,  CAN_HandleTypeDef* bhcan1, CAN_HandleTypeDef* bhcan2, ADC_HandleTypeDef* hadc, SPI_HandleTypeDef* hspi, UART_HandleTypeDef* huart);

HAL_StatusTypeDef BMS_Mode_Normal(BMS_TypeDef* bms);

HAL_StatusTypeDef BMS_Mode_Error(BMS_TypeDef* bms);

HAL_StatusTypeDef BMS_Log_Data(BMS_TypeDef* bms);

HAL_StatusTypeDef BMS_Start_Peripherals(BMS_TypeDef* bms);

HAL_StatusTypeDef BMS_Stop_Peripherals(BMS_TypeDef* bms);

void 			  BMS_Mode_LEDBlink(BMS_TypeDef* bms);

#ifdef __cplusplus
}
#endif

#endif /* INC_BMS_H_ */
