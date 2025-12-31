/**
  ******************************************************************************
  * @file    BMS_TIM_driver.h.h
  * @author  Bartosz Rychlicki

  * @Title   Firmware for BMS Master PCB board's timers
  *
  * @brief   This file contains common defines, flags and macros that are used to provide high quality workflow of timers embedded in BMS Msster.
  *
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 AGH Eko-Energy.
  * All rights reserved.
  *
  ******************************************************************************
  */

#ifndef INC_BMS_TIM_DRIVER_H_
#define INC_BMS_TIM_DRIVER_H_

#pragma once


#ifdef __cplusplus
extern "C" {
#endif

/* Includes ----------------------------------------------------------------------------------  */
#include "BMS_Types.h"


/* Macros ------------------------------------------------------------------------------------  */


/* Functions' prototypes ---------------------------------------------------------------------  */
HAL_StatusTypeDef BMS_TIM_LED_Blink(BMS_StatusTypeDef_e status, TIM_HandleTypeDef* htim);


#endif /* INC_BMS_TIM_DRIVER_H_ */
