/**
  ******************************************************************************
  * @file    BMS_Types.h
  * @author  Bartosz Rychlicki

  * @Title   Typedefs and inclues for BMS firmware
  *
  * @brief   This file contains typedefs definitions and includes that are requirde for correct include path between files.
  *
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 AGH Eko-Energy.
  * All rights reserved.
  *
  ******************************************************************************
  */
/* Typedefs --------------------------------------------------------------------------------  */
/**
  * @brief  BMS's mode status | enumeration type
  */
#ifndef INC_BMS_TYPES_H_
#define INC_BMS_TYPES_H_

/* Includes --------------------------------------------------------------------------------  */

// General
#include "main.h"
#include "string.h"

// Drivers
#include "adc_driver.h"
#include "can_driver.h"
#include "can_id_list.h"


/* Typedefs ----------------------------------------------------------------------------------  */
typedef enum{
	BMS_NORMAL = 0,
	BMS_STANDBY,
	BMS_Error
}BMS_StatusTypeDef_e;


/**
  * @brief  BMS's ADC object type
  */
typedef struct{
	ADC_HandleTypeDef   hadc;														// ADC  handle used in BMS's firmware | measuring data via ADC

	uint16_t 			ADC_voltTempCurr[3];										// ADC's converted value (ready to send via CAN1) buff

	ADC_ChannelsTypeDef  cadc1;														// ADC1's Channels' configurations object
	ADC_BufferTypeDef	 badc1;														// ADC1's Channels' converted value buffer

}ADC_BMSTypeDef;

/**
  * @brief  BMS's CAN object type
  */
typedef struct{

	CAN_HandleTypeDef    bhcan1;														// CAN1 handle used in BMS's firmware | sending   data via CAN1
	CAN_HandleTypeDef    bhcan2;														// CAN2 handle used in BMS's firmware | receiving data via CAN1

	CAN_ScheduledMsgList CAN1_Buff;													// CAN1 frames buffer

	uint8_t 			 CAN2_temperatureCells[7][9];									// CAN2's received value (ready to send via CAN1) buff

}CAN_BMSTypeDef;


/**
  * @brief  BMS object type
  */
typedef struct{


	BMS_StatusTypeDef_e  status;													// current BMS status
	BMS_StatusTypeDef_e  prevStatus;												// previous BMS status in case status change occurred


	TIM_HandleTypeDef   htim;														// TIM  handle used in BMS's firmware | LEDs blinking
	UART_HandleTypeDef  huart1;														// UART handle used in BMS's firmware | logging   data via UART1
	SPI_HandleTypeDef   hspi1;														// SPI  handle used in BMS's firmware | sending   data via NRF905

	CAN_BMSTypeDef 		bmsCAN;														// BMS's CAN custom typedef object
	ADC_BMSTypeDef 		bmsADC;														// BMS's ADC custom typedef object

}BMS_TypeDef;


/* Macros ------------------------------------------------------------------------------------  */

//TIM
#define TIM_GREEN_LD		TIM_CHANNEL_4
#define TIM_RED_LD   		TIM_CHANNEL_3

// ADC
#define ADC_VOLTAGE_CH 12
#define ADC_CURRENT_CH 11
#define ADC_TEMP_CH    10

// STM32F105
#define BMS_VCC_SUPPLY      87.0f
#define VCC_SUPPLY_VOLTAGE  3.3f


#endif /* INC_BMS_TYPES_H_ */
