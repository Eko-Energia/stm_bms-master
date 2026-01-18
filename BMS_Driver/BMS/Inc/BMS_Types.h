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

// Enumeration types
/**
  * @brief  BMS's status definition
  */
typedef enum{

	BMS_NORMAL = 0,																		/*< BMS's normal status>*/
	BMS_Error																			/*< BMS's error  status>*/

}BMS_StatusTypeDef_e;


/**
  * @brief  BMS's NRF905 status definition
  */
typedef enum{

	BMS_NRF905_ON = 0,																	/*< BMS's NRF905 Active     status>*/
	BMS_NRF905_OFF,																		/*< BMS's NRF905 non-active status>*/

}NRF905_StatusTypeDef_e;

/**
  * @brief  BMS's NRF905's SPI status definition
  */
typedef enum{

	BMS_NRF905_SPI_PWRDOWN = 0,															/*< BMS's NRF905's SPI Enabled during NRF905 power down>*/
	BMS_NRF905_SPI_STANDBY,																/*< BMS's NRF905's SPI Enabled during NRF905 standby mode>*/
	BMS_NRF905_SPI_READRXREG,															/*< BMS's NRF905's SPI - read data from RX register>*/
	BMS_NRF905_SPI_SHOCKBURST_RX,														/*< BMS's NRF905's SPI - Radio enabled, ShockBurst™ RX>*/
	BMS_NRF905_SPI_SHOCKBURST_TX														/*< BMS's NRF905's SPI - Radio enabled, ShockBurst™ TX>*/

}NRF905_SPIStatusTypeDef_e;

/**
  * @brief  BMS's NRF905's SPI selected command definition
  */
typedef enum{
	NRF905_CMD_WC  	= 0x00,																/*<Write configuration register>*/
	NRF905_CMD_WR  	= 0x10,																/*<Read  configuration register>*/
	NRF905_CMD_WTP 	= 0x20,																/*<Write TX payload>*/
	NRF905_CMD_RTP  = 0x21,																/*<Read  TX payload>*/
	NRF905_CMD_WTA  = 0x22,																/*<Write TX address>*/
	NRF905_CMD_RTA  = 0x23,																/*<Read  TX address>*/
	NRF905_CMD_RRP  = 0x24,																/*<Read  RX payload>*/
	NRF905_CMD_CC   = 0x80																/*<Special command for fast setting CH_NO, HFREQ_PLL, PA_PWR in CONFIGURATION REGISTER>*/

}NRF905_SPICommandTypeDef;

/**
  * @brief  BMS's NRF905's amplifier status definition
  */
typedef enum{

	BMS_NRF905_AMPLIFIER_SHUTDOWN = 0,													/*< BMS's NRF905's amplifier shutdown mode>*/
	BMS_NRF905_AMPLIFIER_LOWPWR,														/*< BMS's NRF905's amplifier low PWR mode>*/
	BMS_NRF905_AMPLIFIER_MEDIUMPWR,														/*< BMS's NRF905's amplifier medium PWR mode>*/
	BMS_NRF905_AMPLIFIER_HIGHPWR														/*< BMS's NRF905's amplifier high PWR mode>*/

}NRF905_AmplifierStatusTypeDef_e;

// Structures and unions

/**
  * @brief  BMS's ADC object type
  */
typedef struct{

	ADC_HandleTypeDef   hadc;															/*< ADC  handle used in BMS's firmware | measuring data via ADC>*/

	uint16_t 			ADC_voltTempCurr[3];											/*< ADC's converted value (ready to send via CAN1) buff>*/

	ADC_ChannelsTypeDef cadc1;															/*< ADC1's Channels' configurations object>*/
	ADC_BufferTypeDef	badc1;															/*< ADC1's Channels' converted value buffer>*/

}ADC_BMSTypeDef;

/**
  * @brief  BMS's CAN object type
  */
typedef struct{

	CAN_HandleTypeDef    bhcan1;														/*< CAN1 handle used in BMS's firmware | sending   data via CAN1>*/
	CAN_HandleTypeDef    bhcan2;														/*< CAN2 handle used in BMS's firmware | receiving data via CAN1>*/

	CAN_ScheduledMsgList CAN1_Buff;														/*< CAN1 frames buffer>*/

	uint8_t 			 CAN2_temperatureCells[7][9];									/*< CAN2's received value (ready to send them via CAN1) buff>*/

}CAN_BMSTypeDef;


/**
  * @brief  NRF905's object  RF-configuration typedef
  */
typedef struct{

	uint32_t RX_ADDRESS;

	uint16_t CH_NO       : 9;

	uint8_t  HFREQ_PLL   : 1;
	uint8_t  PA_PWR      : 2;
	uint8_t  RX_RED_PWR  : 1;
	uint8_t  AUTO_RETRAN : 1;
	uint8_t  RX_AFW      : 3;
	uint8_t  TX_AFW      : 3;
	uint8_t  RX_PW       : 6;
	uint8_t  TX_PW       : 6;
	uint8_t  UP_CLK_FREQ : 2;
	uint8_t  UP_CLK_EN   : 1;
	uint8_t  XOF         : 3;
	uint8_t  CRC_EN		 : 1;
	uint8_t  CRC_MODE    : 1;

	void		(*GetRFConfig)(uint8_t * data);

}NRF905_RFConfigurationTypeDef;

/**
  * @brief  NRF905's object typedef
  */
typedef struct{
	NRF905_AmplifierStatusTypeDef_e status;
}NRF905_AmplifierTypeDef;



/**
  * @brief  NRF905's SPI controller object typedef
  */
typedef struct{

	NRF905_SPIStatusTypeDef_e  	  status;											/*< NRF905's SPI controller status>*/
	NRF905_SPICommandTypeDef 	  selectedCommand;									/*< NRF905's SPI selected CMD for operations>*/

}NRF905_SPITypeDef;

/**
  * @brief  NRF905 object typedef
  */
typedef struct{
	NRF905_AmplifierTypeDef       amplifier;									    /*< NRF905's amplifier object>*/
	NRF905_SPITypeDef		      spi;												/*< NRF905's SPI object>*/
	NRF905_StatusTypeDef_e     	  status;										    /*< NRF905's status>*/

	NRF905_RFConfigurationTypeDef nrfConfig;								        /*< NRF905's configuration>*/

	uint32_t 					  selectedAddr;

	void						  (*GetPayload)(uint8_t* data);

}NRF905_TypeDef;


/**
  * @brief  BMS object type
  */
typedef struct{


	BMS_StatusTypeDef_e  status;													/*< Current BMS status>*/
	BMS_StatusTypeDef_e  prevStatus;												/*< Previous BMS status in case status change occurred>*/


	UART_HandleTypeDef  huart1;														/*< UART handle used in BMS's firmware | logging   data via UART1>*/
	SPI_HandleTypeDef   hspi1;														/*< SPI  handle used in BMS's firmware | sending   data via NRF905>*/

	CAN_BMSTypeDef 		bmsCAN;														/*< BMS's CAN 	 custom typedef object>*/
	ADC_BMSTypeDef 		bmsADC;														/*< BMS's ADC 	 custom typedef object>*/
	NRF905_TypeDef      bmsNRF905;													/*< BMS's NRF905 custom typedef object>*/

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
