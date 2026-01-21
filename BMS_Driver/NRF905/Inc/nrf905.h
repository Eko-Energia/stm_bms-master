/**
  ******************************************************************************
  * @file    nrf905.h
  * @author  Bartosz Rychlicki
  * @author  AGH Eko-Energia

  * @Title   Firmware for BMS's nrf905 module, which provides radio communication
  *
  * @brief   This file contains common macros definitions, function's prototypes and flags of SPI's communication status
  *
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 AGH Eko-Energy.
  * All rights reserved.
  *
  ******************************************************************************
  */

#ifndef INC_NRF905_H_
#define INC_NRF905_H_

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* Includes --------------------------------------------------------------------------------  */
#include "BMS_Types.h"

/* Macros ----------------------------------------------------------------------------------  */

/* Variables ----------------------------------------------------------------------------------  */
extern BMS_TypeDef bms;

/* Functions Prototypes --------------------------------------------------------------------  */
// NRF905 power modes
HAL_StatusTypeDef BMS_NRF905_ChangeMode(BMS_TypeDef* bms, NRF905_StatusTypeDef_e status);


// NRF905 operations
void NRF905_Init(void);

void NRF905_RF_Config(void);

HAL_StatusTypeDef NRF905_Write_TX_Payload(BMS_TypeDef* bms);

HAL_StatusTypeDef NRF905_Write_TX_Address(BMS_TypeDef* bms);

HAL_StatusTypeDef NRF905_Send(BMS_TypeDef* bms, uint32_t Id, uint8_t* data, uint8_t DLC);

void 			  NRF905_GetConfigData(uint8_t* data);

// NRF905's SPI operations
HAL_StatusTypeDef BMS_NRF905_SPI_ChangeCMD(BMS_TypeDef* bms);

HAL_StatusTypeDef BMS_NRF905_SPI_TransferReceive(BMS_TypeDef* bms, uint8_t* txByte, uint8_t* rxByte);

HAL_StatusTypeDef BMS_NRF905_WriteReg(BMS_TypeDef* bms, uint8_t cmd, uint8_t* data, uint8_t len);

HAL_StatusTypeDef BMS_NRF905_ReadReg(BMS_TypeDef* bms, uint8_t cmd, uint8_t* data, uint8_t len);



// NRF905's amplifier power modes
HAL_StatusTypeDef BMS_NRF905_Amplifier_ChangeMode(BMS_TypeDef* bms, NRF905_AmplifierStatusTypeDef_e status);

#endif /* INC_NRF905_H_ */







