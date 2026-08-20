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
  * Copyright (c) 2026 AGH Eko-Energy.
  * All rights reserved.
  *
  ******************************************************************************
  */

#ifndef INC_BMS_TYPES_H_
#define INC_BMS_TYPES_H_

/* Includes --------------------------------------------------------------------------------  */

/* General */
#include "main.h"
#include <string.h>

/* Drivers */
#include "adc_driver.h"
#include "can_driver.h"
#include "error_handler.h"
#include "pwm_driver.h"

/* Typedefs ----------------------------------------------------------------------------------  */

/*
	 ==============================================================================
						   ##### ENUMERATIONS #####
	 ==============================================================================
*/

/**
  * @brief  FAN control status definition
  */
typedef enum{

	OFF  = 0,																		/*< BMS's FAN OFF status>*/
	ON																				/*< BMS's FAN ON status>*/

}FAN_StatusTypeDef_e;

/**
  * @brief  BMS operating mode status definition
  */
typedef enum{

	BMS_NORMAL = 0,																		/*< BMS's normal status>*/
	BMS_Error																			/*< BMS's error  status>*/

}BMS_StatusTypeDef_e;

/**
  * @brief  BMS's PWM status definition
  */
typedef enum{

	PWM_Startup = 0,																	/*< BMS's PWM module state on FW startup >*/
	PWM_Operational																		/*< BMS's PWM state in operational (working stage) >*/

}PWM_BMSStatusTypeDef;

/*
	 ==============================================================================
						   ##### STRUCTURES #####
	 ==============================================================================
*/

/**
  * @brief Structure of BMS's PWM module
  */
typedef struct{

	PWM_BMSStatusTypeDef    status;														/*< BMS's PWM module status>*/
	struct PWM_Out_signal  htim;														/*< BMS's handle to PWM driver's struct>*/

}PWM_BMSTypeDef;

/**
  * @brief  BMS's ADC object type
  */
typedef struct{
	ADC_HandleTypeDef*           hadc;													/*<ADC  handle used in BMS's firmware | measuring data via ADC>*/

	uint16_t 			         ADC_voltTempCurr[3];									/*<ADC's converted value (ready to send via CAN1) buff>*/

	ADC_ChannelsConfigTypeDefs   cadc1;													/*<ADC1's Channels' configurations object>*/
	ADC_BufferTypeDef	         badc1;													/*<ADC1's Channels' converted value buffer>*/

}ADC_BMSTypeDef;

/**
  * @brief  BMS's CAN object type
  */
typedef struct{

	CAN_HandleTypeDef*   bhcan1;													/*<CAN1 handle used in BMS's firmware | sending   data via CAN1>*/
	CAN_HandleTypeDef*   bhcan2;													/*<CAN2 handle used in BMS's firmware | receiving data via CAN2>*/

	struct CAN_scheduledMsgList CAN1_Buff;											/*<CAN1 frames buffer>*/

	uint8_t 			 CAN2_temperatureCells[7][9];								/*<CAN2's received value (ready to send via CAN1) buff>*/

}CAN_BMSTypeDef;


/**
  * @brief  BMS error logger object type
  */
typedef struct{

	UART_HandleTypeDef  *huart1;															/*< UART handle used in BMS's firmware | logging   data via UART1>*/

	char* loggerMessage;																	/*< Error logger's message>*/

}BMS_ErrorLoggerTypeDef;

/**
  * @brief  Main BMS object type aggregating peripherals and runtime state
  */
typedef struct{


	BMS_StatusTypeDef_e    status;															/*< Current BMS status>*/
	BMS_StatusTypeDef_e    prevStatus;														/*< Previous BMS status in case status change occurred>*/

	BMS_ErrorLoggerTypeDef errorLogger;														/*< Error logger typedef's object>*/

	CAN_BMSTypeDef 		   bmsCAN;															/*< BMS's CAN 	 custom typedef object>*/
	ADC_BMSTypeDef 		   bmsADC;															/*< BMS's ADC 	 custom typedef object>*/

	EH_HandleTypeDef       beh;																/*< BMS's handle to error handler >*/

	PWM_BMSTypeDef		   bpwm;															/*< BMS's PWM custom typedef object>*/

	uint8_t                safeStateStatus;													/*< Variable stores safe state status>*/

	FAN_StatusTypeDef_e	   fanState;														/*<	Variable stores info of fan status ON/OFF>*/

	float 				   maxTemperature;													/*< Variable that stores max read temperature>*/

}BMS_TypeDef;


/* Macros ------------------------------------------------------------------------------------  */

/*
	 ==============================================================================
						   ##### BUILD / TEST CONTROL #####
	 ==============================================================================
*/

/* Test Control macro - uncomment when needed */
//#define PROD (0x01) 																       /*<Macro that enables FW to trigger error handler in some scenarios - for testing purposes it's commented>*/

/*
	 ==============================================================================
						   ##### BMS GENERAL #####
	 ==============================================================================
*/

#define BMS_NODE 		   (2)																/*< BMS node identifier for error handler>*/
#define TEMP_MAX           (60)																/*< Maximum allowed measured temperature [C]>*/
#define TEMP_MIN           (0)																/*< Minimum allowed measured temperature [C]>*/
#define BMS_VCC_SUPPLY     (87.0f)														   /*<MAX DC Supply Voltage of BMS Master PCB>*/

/*
	 ==============================================================================
						   ##### ADC CHANNELS #####
	 ==============================================================================
*/

/**
  * @brief ADC Channels' numbers macros definitions
  */
#define ADC_VOLTAGE_CH 		(12)															/*<12th channel of ADC1>*/
#define ADC_CURRENT_CH 		(11)															/*<11th channel of ADC1>*/
#define ADC_TEMP_CH    		(10)															/*<10th channel of ADC1>*/

/*
	 ==============================================================================
						   ##### ERROR HANDLER CODES #####
	 ==============================================================================
*/

#define EH_BMS_TEMP_HIGH    (0x100)															/*<Error code for reporting too high temperature on BMS battery>*/
#define EH_CAN2_TEMP_HIGH   (0x200)															/*<Error code for reporting too high temperature on cells'>*/
#define EH_SAFE_STATE_LEAK  (0x300)															/*<Error code for reporting incorrect safe state behavior>*/

/*
	 ==============================================================================
						   ##### SAFE STATE #####
	 ==============================================================================
*/

#define SAFE_STATE_OK		(0x00)															/*<Safe state ok status>*/
#define SAFE_STATE_ERROR    (0x01)															/*<Safe state error status>*/

/*
	 ==============================================================================
						   ##### COOLING THRESHOLDS #####
	 ==============================================================================
*/

#define PRE_COOLING_TEMP    (50.0f)															/*< Upper threshold that triggers cooling>*/
#define POST_COOLING_TEMP   (40.0f)															/*< Lower threshold that disables cooling>*/

/*
	 ==============================================================================
						   ##### MCU SUPPLY #####
	 ==============================================================================
*/

#define VCC_SUPPLY_VOLTAGE  (3.3f)															/*<MAX DC Supply Voltage for STM32f105>*/

/*
	 ==============================================================================
						   ##### BMS Resistors in voltage divider#####
	 ==============================================================================
*/
#define R1					(249000.0f)
#define R2					(9100.0f)

/* Fixed 10 kΩ from ADC node (PC0) to GND; NTC is the upper leg to VCC */
#define NTC_LOWER_OHM		(10000.0f)

#endif /* INC_BMS_TYPES_H_ */
