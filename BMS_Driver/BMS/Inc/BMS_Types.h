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
