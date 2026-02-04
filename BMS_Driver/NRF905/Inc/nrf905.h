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
#define NRF905_ADDR_SIZE    (4)					/*<Size of address frame, sent via SPI>*/
#define NRF905_PAYLOAD_SIZE (32)				/*<Size of payload frame, sent via SPI>*/

#define NRF905_ADDR_MSB_pos  (24U)				/*<Byte0 position in 32-bit address>*/
#define NRF905_ADDR_NMSB_pos (16U)				/*<Byte1 position in 32-bit address>*/
#define NRF905_ADDR_NLSB_pos (8U)				/*<Byte2 position in 32-bit address>*/
#define NRF905_ADDR_LSB_pos  (0U)				/*<Byte3 position in 32-bit address>*/


/* Variables ----------------------------------------------------------------------------------  */


/* Functions Prototypes --------------------------------------------------------------------  */
// NRF905 power modes
HAL_StatusTypeDef 	 BMS_NRF905_ChangeMode(BMS_TypeDef* bms, NRF905_StatusTypeDef_e status);

// NRF905 operations

HAL_StatusTypeDef    BMS_NRF905_Init(BMS_TypeDef* bms);

HAL_StatusTypeDef 	 BMS_NRF905_Mode_Normal(BMS_TypeDef* bms);

HAL_StatusTypeDef 	 BMS_NRF905_Set_NormalMode(BMS_TypeDef* bms);

HAL_StatusTypeDef 	 BMS_NRF905_Set_ErrorMode(BMS_TypeDef* bms);

HAL_StatusTypeDef 	 BMS_NRF905_Transmit(BMS_TypeDef* bms, uint32_t* id);

void 			  	 BMS_NRF905_GetConfigData(uint8_t* data);

HAL_StatusTypeDef 	 BMS_NRF905_SelectPayload(BMS_TypeDef* bms, uint32_t* id, uint8_t* payload);

HAL_StatusTypeDef 	 BMS_NRF905_ExtractAddr(BMS_TypeDef* bms, uint8_t* selectedAddr, uint8_t* targetAddr);

// NRF905's SPI operations
HAL_StatusTypeDef 	 BMS_NRF905_SPI_ChangeMode(BMS_TypeDef* bms, NRF905_SPIStatusTypeDef_e status);

HAL_StatusTypeDef 	 BMS_NRF905_SPI_ChangeCMD(BMS_TypeDef* bms);

HAL_StatusTypeDef 	 BMS_NRF905_SPI_TransferReceive(BMS_TypeDef* bms, uint8_t* txByte, uint8_t* rxByte);

HAL_StatusTypeDef 	 BMS_NRF905_WriteReg(BMS_TypeDef* bms, uint8_t cmd, uint8_t* data, uint8_t len);

HAL_StatusTypeDef 	 BMS_NRF905_ReadReg(BMS_TypeDef* bms, uint8_t cmd, uint8_t* data, uint8_t len);



// NRF905's amplifier power modes
HAL_StatusTypeDef 	 BMS_NRF905_Amplifier_ChangeMode(BMS_TypeDef* bms, NRF905_AmplifierStatusTypeDef_e status);

#endif /* INC_NRF905_H_ */







