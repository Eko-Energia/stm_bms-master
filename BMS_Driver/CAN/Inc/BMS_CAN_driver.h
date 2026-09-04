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

/* Variables -------------------------------------------------------------------------------  */

/* Macros ----------------------------------------------------------------------------------  */

/*
	 ==============================================================================
						   ##### CAN1 TX SCALING #####
	 ==============================================================================
*/

/* CAN1 Scaling parameters */
#define VOLTAGE_OFFSET          (0.0f)													/*< Voltage scaling offset>*/
#define VOLTAGE_GAIN 		    (0.1f)													/*< Voltage scaling gain>*/

#define CURRENT_OFFSET          (0.0f)												/*< Current scaling offset>*/
#define CURRENT_GAIN 		    (0.1f)													/*< Current scaling gain>*/

#define TEMPERATURE_OFFSET      (0.0f)													/*< Temperature scaling offset>*/
#define TEMPERATURE_GAIN        (0.01f)													/*< Temperature scaling gain>*/

/*
	 ==============================================================================
						   ##### CAN2 RX SCALING #####
	 ==============================================================================
*/

/* CAN2 scaling params */
#define THERM_TEMPERATURE_GAIN  (0.39216f)												/*< Thermistor temperature gain for CAN2 payloads>*/

/*
	 ==============================================================================
						   ##### CAN FRAME DLC #####
	 ==============================================================================
*/

/* CAN Frames DLCs */
#define BMS_NODE_DLC		    (8)														/*< DLC of BMS node identification frame>*/
#define BMS_ID1_DLC			    (8)														/*< DLC of StdId=1 bring-up / heartbeat frame>*/
#define BMS_VOLTCURTEMP_DLC     (6)														/*< DLC of voltage/current/temperature frame>*/
#define BMS_THERMx_DLC		    (7)														/*< DLC of thermistor group frames>*/
#define BMS_JK_PACK_INFO_DLC    (8)														/*< DLC of JK pack info (0x140)>*/
#define BMS_JK_CELL_VOLT_DLC    (8)														/*< DLC of JK cell voltage block (4x u16)>*/
#define BMS_JK_TEMP_DLC         (8)														/*< DLC of JK MOS/bal temperature frame>*/
#define BMS_JK_CYCLE_STATS_DLC  (8)														/*< DLC of JK cycle stats frame>*/

/*
	 ==============================================================================
						   ##### CAN FRAME PERIODS #####
	 ==============================================================================
*/

/* CAN Frames Periods [ms] */
#define BMS_NODE_PERIOD		    (5000)													/*< Period of BMS node frame>*/
#define BMS_ID1_PERIOD		    (100)													/*< Period of StdId=1 bring-up / heartbeat frame>*/
#define BMS_VOLTCURTEMP_PERIOD  (500)													/*< Period of voltage/current/temperature frame>*/
#define BMS_THERMx_PERIOD		(1000)													/*< Period of thermistor group frames>*/
#define BMS_JK_CAN_PERIOD       (1000)													/*< Period of JK telemetry export frames (docs/BMS-JK.md)>*/

/*
	 ==============================================================================
						   ##### CAN FRAME IDs #####
	 ==============================================================================
*/

/* CAN Frames IDs */
#define BMS_ID1_ID		        (1)														/*< StdId=1 bring-up / heartbeat TX frame ID>*/
#define SAFE_STATE_ID			(BMS_ID1_ID)											/*< Safe-state RX status frame ID (same StdId as bring-up TX)>*/
#define BMS_NODE_ID		        (128)													/*< BMS node identification frame ID>*/
#define BMS_VOLTCURTEMP_ID      (130)													/*< Voltage/current/temperature frame ID>*/
#define BMS_THERM1_ID		    (131)													/*< Thermistor group 1 frame ID>*/
#define BMS_THERM2_ID		    (132)													/*< Thermistor group 2 frame ID>*/
#define BMS_THERM3_ID			(133)													/*< Thermistor group 3 frame ID>*/
#define BMS_THERM4_ID			(134)													/*< Thermistor group 4 frame ID>*/
#define BMS_THERM5_ID			(135)													/*< Thermistor group 5 frame ID>*/
#define BMS_THERM6_ID			(136)													/*< Thermistor group 6 frame ID>*/
#define BMS_THERM7_ID			(137)													/*< Thermistor group 7 frame ID>*/
#define BMS_THERM8_ID			(138)													/*< Thermistor group 8 frame ID>*/
#define BMS_THERM9_ID			(139)													/*< Thermistor group 9 frame ID>*/

/* JK BMS telemetry export (docs/BMS-JK.md) — separate from ADC/therm IDs */
#define BMS_JK_PACK_INFO_ID     (0x140U)												/*< Pack V/I/SOC/SOH/status/mode>*/
#define BMS_JK_CELL_VOLT_1_4_ID (0x141U)												/*< Cells 1..4 voltages [mV]>*/
#define BMS_JK_CELL_VOLT_5_8_ID (0x142U)												/*< Cells 5..8 voltages [mV]>*/
#define BMS_JK_CELL_VOLT_9_12_ID (0x143U)												/*< Cells 9..12 voltages [mV]>*/
#define BMS_JK_TEMP_ID          (0x144U)												/*< MOS / bal temperature [°C]>*/
#define BMS_JK_CYCLE_STATS_ID   (0x148U)												/*< Cycle count + cellCount>*/

/*
	 ==============================================================================
						   ##### CAN2 THERM SCAN / FILTER #####
	 ==============================================================================
*/

#define BMS_THERM_PCB_COUNT     (7)														/*< Number of slave PCBs providing thermistors>*/
#define BMS_THERM_PER_PCB       (9)														/*< Thermistors per slave PCB>*/
#define BMS_THERM_TOTAL         (40)													/*< Full unique therm set for FAN max latch>*/
#define BMS_THERM_ID_BASE       (200)													/*< CAN2 therm ID = BASE + pcb*10 + therm>*/
#define BMS_CAN2_THERM_FILTER_ID_LO    (0x0C0U)											/*< HW bank 15 ID: 0x0C0..0x0FF covers decimal 211..255>*/
#define BMS_CAN2_THERM_FILTER_MASK_LO  (0x7C0U)											/*< Mask for bank 15 (bits 10:6)>*/
#define BMS_CAN2_THERM_FILTER_ID_HI    (0x100U)											/*< HW bank 16 ID: 0x100..0x11F covers decimal 256..279>*/
#define BMS_CAN2_THERM_FILTER_MASK_HI  (0x7E0U)											/*< Mask for bank 16 (bits 10:5); 280..287 dropped in SW>*/


/* Functions' prototypes ------------------------------------------------------------------  */

/*
	 ==============================================================================
						   ##### CAN INIT / TX SETUP #####
	 ==============================================================================
*/

/*
  * @brief  Initializes CAN1/CAN2 transceivers, stacks and scheduled TX frames
  * @param  bms Pointer to BMS handle
  * @retval HAL_OK on success, HAL_ERROR on failure
  */
HAL_StatusTypeDef BMS_CAN_Init(BMS_TypeDef* bms);

/*
  * @brief  Adds a scheduled CAN TX message and binds its getData callback by ID
  * @param  bms    Pointer to BMS handle
  * @param  Id     Standard CAN identifier
  * @param  DLC    Data length code
  * @param  period Transmission period [ms]
  * @retval HAL_OK on success, HAL_ERROR on failure
  */
HAL_StatusTypeDef BMS_CAN_AddMessage(BMS_TypeDef* bms, uint32_t Id, uint8_t DLC, uint32_t period);

/*
  * @brief  Registers all BMS peripheral TX frames in the scheduled message list
  * @param  bms Pointer to BMS handle
  * @retval HAL_OK on success, HAL_ERROR on failure
  */
HAL_StatusTypeDef BMS_CAN_AddPeripheralFrames(BMS_TypeDef* bms);

/*
	 ==============================================================================
						   ##### CAN TX PAYLOAD PACKERS #####
	 ==============================================================================
*/

/*
  * @brief  Packs StdId=1 bring-up / heartbeat TX payload (rolling counter)
  * @param  data    Output payload buffer
  * @param  context Pointer to BMS context (unused)
  * @retval None
  */
void 			  BMS_CAN_Get_ID1_Data(uint8_t *data, void *context);

/*
  * @brief  Packs ADC voltage/current/temperature into CAN TX payload
  * @param  data    Output payload buffer
  * @param  context Pointer to BMS context (cast to BMS_TypeDef*)
  * @retval None
  */
void 			  BMS_CAN_Get_ADC_Data(uint8_t *data, void *context);

/*
  * @brief  Packs one thermistor group from CAN2 RX buffer into CAN1 TX payload
  * @param  data    Output payload buffer
  * @param  thermId Thermistor group index (0..8)
  * @retval None
  */
void 			  BMS_CAN_PackCAN2Temps(uint8_t* data, uint8_t thermId);

/*
  * @brief  getData callback for thermistor group 1 TX frame
  * @param  data    Output payload buffer
  * @param  context Pointer to BMS context
  * @retval None
  */
void 			  BMS_CAN_Get_CAN2_Data_Therm1(uint8_t *data, void* context);

/*
  * @brief  getData callback for thermistor group 2 TX frame
  * @param  data    Output payload buffer
  * @param  context Pointer to BMS context
  * @retval None
  */
void 			  BMS_CAN_Get_CAN2_Data_Therm2(uint8_t *data, void* context);

/*
  * @brief  getData callback for thermistor group 3 TX frame
  * @param  data    Output payload buffer
  * @param  context Pointer to BMS context
  * @retval None
  */
void 			  BMS_CAN_Get_CAN2_Data_Therm3(uint8_t *data, void* context);

/*
  * @brief  getData callback for thermistor group 4 TX frame
  * @param  data    Output payload buffer
  * @param  context Pointer to BMS context
  * @retval None
  */
void 			  BMS_CAN_Get_CAN2_Data_Therm4(uint8_t *data, void* context);

/*
  * @brief  getData callback for thermistor group 5 TX frame
  * @param  data    Output payload buffer
  * @param  context Pointer to BMS context
  * @retval None
  */
void 			  BMS_CAN_Get_CAN2_Data_Therm5(uint8_t *data, void* context);

/*
  * @brief  getData callback for thermistor group 6 TX frame
  * @param  data    Output payload buffer
  * @param  context Pointer to BMS context
  * @retval None
  */
void 			  BMS_CAN_Get_CAN2_Data_Therm6(uint8_t *data, void* context);

/*
  * @brief  getData callback for thermistor group 7 TX frame
  * @param  data    Output payload buffer
  * @param  context Pointer to BMS context
  * @retval None
  */
void 			  BMS_CAN_Get_CAN2_Data_Therm7(uint8_t *data, void* context);

/*
  * @brief  getData callback for thermistor group 8 TX frame
  * @param  data    Output payload buffer
  * @param  context Pointer to BMS context
  * @retval None
  */
void 			  BMS_CAN_Get_CAN2_Data_Therm8(uint8_t *data, void* context);

/*
  * @brief  getData callback for thermistor group 9 TX frame
  * @param  data    Output payload buffer
  * @param  context Pointer to BMS context
  * @retval None
  */
void 			  BMS_CAN_Get_CAN2_Data_Therm9(uint8_t *data, void* context);

/*
  * @brief  Packs JK snapshot pack voltage/current/SOC into CAN TX payload (0x140)
  */
void 			  BMS_CAN_Get_JK_PackInfo(uint8_t *data, void *context);

/*
  * @brief  Packs JK cell voltages 1..4 [mV] little-endian (0x141)
  */
void 			  BMS_CAN_Get_JK_CellVolt_1_4(uint8_t *data, void *context);

/*
  * @brief  Packs JK cell voltages 5..8 [mV] little-endian (0x142)
  */
void 			  BMS_CAN_Get_JK_CellVolt_5_8(uint8_t *data, void *context);

/*
  * @brief  Packs JK cell voltages 9..12 [mV] little-endian (0x143)
  */
void 			  BMS_CAN_Get_JK_CellVolt_9_12(uint8_t *data, void *context);

/*
  * @brief  Packs JK MOS/bal temperatures [°C] (0x144)
  */
void 			  BMS_CAN_Get_JK_Temp(uint8_t *data, void *context);

/*
  * @brief  Packs JK cycle count and cellCount (0x148)
  */
void 			  BMS_CAN_Get_JK_CycleStats(uint8_t *data, void *context);

/*
	 ==============================================================================
						   ##### CAN HELPERS / RX #####
	 ==============================================================================
*/

/*
  * @brief  Extracts most significant byte from 16-bit value
  * @param  value 16-bit input value
  * @retval MSB as uint8_t
  */
uint8_t 		  BMS_CAN_GetMSB(uint16_t value);

/*
  * @brief  Extracts least significant byte from 16-bit value
  * @param  value 16-bit input value
  * @retval LSB as uint8_t
  */
uint8_t 		  BMS_CAN_GetLSB(uint16_t value);

/*
  * @brief  Applies channel-specific gain/offset and stores scaled value in ADC TX buffer
  * @param  bms     Pointer to BMS handle
  * @param  channel ADC channel identifier (voltage/temp/current)
  * @param  value_f Pointer to engineering-unit value to scale
  * @retval HAL_OK on success, HAL_ERROR on failure
  */
HAL_StatusTypeDef BMS_CAN_ScallingParams(BMS_TypeDef* bms, uint8_t channel, float value_f);

/*
  * @brief  Handles received CAN2 frames (temps / safe-state) and updates BMS state
  * @param  bms Pointer to BMS handle
  * @retval HAL_OK on success, HAL_ERROR on failure
  */
HAL_StatusTypeDef BMS_CAN_HandleRxMsg(BMS_TypeDef* bms);

/*
  * @brief  HAL RX FIFO0 callback — marks pending CAN2 reception
  * @param  hcan Pointer to CAN HAL handle that triggered the interrupt
  * @retval None
  */
void 			  HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan);

#ifdef __cplusplus
}
#endif

#endif	/* INC_BMS_CAN_DRIVER_H_ */
