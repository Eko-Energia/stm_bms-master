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

/* Variables ----------------------------------------------------------------------------------  */
extern uint32_t lastTick;

/* Functions Prototypes --------------------------------------------------------------------  */
HAL_StatusTypeDef BMS_Init(BMS_TypeDef* bms,  CAN_HandleTypeDef* bhcan1, CAN_HandleTypeDef* bhcan2, TIM_HandleTypeDef* htim);

HAL_StatusTypeDef BMS_Mode_Normal(BMS_TypeDef* bms, TIM_HandleTypeDef* htim, ADC_HandleTypeDef* hadc);

HAL_StatusTypeDef BMS_Mode_Error(BMS_TypeDef* bms, TIM_HandleTypeDef* htim);

HAL_StatusTypeDef BMS_Log_Data(BMS_TypeDef* bms);

void BMS_Mode_LEDBlink(BMS_TypeDef* bms);


#endif /* INC_BMS_H_ */
