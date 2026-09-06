/**
  ******************************************************************************
  * @file    BMS_ADC_driver.h
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

/* IIR on pack volts; 1/32 holds hundredths without lagging DC */
#define BMS_VPACK_LP_ALPHA   (1.0f / 32.0f)

/* Hold displayed value until change exceeds this — stops 82.19/82.21 flicker */
#define BMS_VPACK_HOLD_V     (0.015f)

/*
 * Linear divider reads ~56 V while the meter is 82.2 V (1.5-cycle S&H).
 * Scale the divider once; do not use the quadratic — at ~2 V pin it
 * jumps to ~199 V from 99.725*Vpin alone.
 */
#define BMS_VPACK_METER_V    (82.2f)
#define BMS_VPACK_DIVIDER_V  (56.0f)
#define BMS_VPACK_SCALE      (BMS_VPACK_METER_V / BMS_VPACK_DIVIDER_V)

/* Functions' prototypes ---------------------------------------------------------------------  */

/*
	 ==============================================================================
						   ##### ADC INIT / READ #####
	 ==============================================================================
*/

/*
  * @brief  Initializes BMS ADC peripheral (channels config and conversion buffer)
  * @param  bms Pointer to BMS handle
  * @retval HAL_OK on success, HAL_ERROR on failure
  */
HAL_StatusTypeDef BMS_ADC_Init(BMS_TypeDef* bms);

/*
  * @brief  Reads all BMS ADC channels (voltage, temperature, current)
  * @param  bms Pointer to BMS handle
  * @retval HAL_OK on success, HAL_ERROR on failure
  */
HAL_StatusTypeDef BMS_ADC_ReadValues(BMS_TypeDef* bms);

/*
  * @brief  Reads pack voltage from ADC, applies divider and CAN scaling
  * @param  bms Pointer to BMS handle
  * @retval HAL_OK on success, HAL_ERROR on failure
  */
HAL_StatusTypeDef BMS_ADC_Read_Voltage(BMS_TypeDef* bms);

/*
  * @brief  Reads NTC temperature from ADC, converts via LUT and applies CAN scaling
  * @param  bms Pointer to BMS handle
  * @retval HAL_OK on success, HAL_ERROR on failure
  */
HAL_StatusTypeDef BMS_ADC_Read_Temperature(BMS_TypeDef* bms);

/*
  * @brief  Reads current from ADC, converts to engineering units and applies CAN scaling
  * @param  bms Pointer to BMS handle
  * @retval HAL_OK on success, HAL_ERROR on failure
  */
HAL_StatusTypeDef BMS_ADC_Read_Current(BMS_TypeDef* bms);

/*
	 ==============================================================================
						   ##### NTC HELPERS #####
	 ==============================================================================
*/

/*
  * @brief  Calibrates measured NTC temperature value
  * @param  measured Raw / interpolated temperature measurement
  * @retval Calibrated temperature value
  */
float BMS_ADC_NTC_calibrateTemperature(float measured);

/*
  * @brief  Converts NTC resistance to temperature using R-T lookup table + interpolation
  * @param  Rt Measured NTC resistance [Ohm]
  * @retval Temperature in Celsius, or -1000.0f on lookup error
  */
float BMS_ADC_NTC_GetTemperature(float Rt);


#ifdef __cplusplus
}
#endif

#endif /* INC_BMS_ADC_DRIVER_H_ */
