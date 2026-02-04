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
  * @brief  BMS's NRF905's amplifier status definition
  */
typedef enum{

	BMS_NRF905_AMPLIFIER_SHUTDOWN = 0,													/*< BMS's NRF905's amplifier shutdown mode>*/
	BMS_NRF905_AMPLIFIER_LOWPWR,														/*< BMS's NRF905's amplifier low PWR mode>*/
	BMS_NRF905_AMPLIFIER_MEDIUMPWR, 													/*< BMS's NRF905's amplifier medium PWR mode>*/
	BMS_NRF905_AMPLIFIER_HIGHPWR														/*< BMS's NRF905's amplifier high PWR mode>*/

}NRF905_AmplifierStatusTypeDef_e;

// Structures and unions

/**
  * @brief  BMS's ADC object type
  */
typedef struct{

	ADC_HandleTypeDef   *hadc;															/*< ADC  handle used in BMS's firmware | measuring data via ADC>*/

	uint16_t 			ADC_voltTempCurr[3];											/*< ADC's converted value (ready to send via CAN1) buff>*/

	ADC_ChannelsTypeDef cadc1;															/*< ADC1's Channels' configurations object>*/
	ADC_BufferTypeDef	badc1;															/*< ADC1's Channels' converted value buffer>*/

}ADC_BMSTypeDef;

/**
  * @brief  BMS's CAN object type
  */
typedef struct{

	CAN_HandleTypeDef    *bhcan1;														/*< CAN1 handle used in BMS's firmware | sending   data via CAN1>*/
	CAN_HandleTypeDef    *bhcan2;														/*< CAN2 handle used in BMS's firmware | receiving data via CAN1>*/

	CAN_ScheduledMsgList CAN1_Buff;														/*< CAN1 frames buffer>*/

	uint8_t 			 CAN2_temperatureCells[7][9];									/*< CAN2's received value (ready to send them via CAN1) buff>*/

}CAN_BMSTypeDef;


/**
  * @brief  NRF905's object  RF-configuration typedef
  */
typedef struct{

	uint32_t RX_ADDRESS;																/*<RX address (default: E7E7E7E7),
																						   used bytes depend on RX_AFW						>*/

	uint16_t CH_NO       : 9;															/*<Sents center frequency (default: 108):
																							fr = (422.4+CH_NO/100)(1 + HFREQ_PLL)    		>*/

	uint8_t  HFREQ_PLL   : 1;															/*<Sets PLL in 433MHz or 868/915 MHz mode (default: 0)
																							- 0 - Chip operating in 433 MHz mode
																							- 1 - Chip operating in 868/915 MHz mode 		>*/

	uint8_t  PA_PWR      : 2;															/*<Output power (default: 00):
																							- 00 - -10dBm
																							- 01 - -2dBm
																							- 10 - +6dBm
																							- 11 - +10dBm 							 		>*/

	uint8_t  RX_RED_PWR  : 1;															/*<Reduces current in RX Mode by 1.6mA (default: 0):
																							- 0 - Normal operation
																							- 1 - Reduced power								>*/

	uint8_t  AUTO_RETRAN : 1;															/*<Auto retransmission in ShockBurst TX(default: 0):
																							- 0 - No retransmission
																							- 1 - Retransmission of data packet				>*/

	uint8_t  RX_AFW      : 3;															/*<RX address width (default: 100):
																							- 001 - 1 byte  RX address width
																							- 100 - 4 bytes RX address width				>*/

	uint8_t  TX_AFW      : 3;															/*<TX address width (default: 100):
																							- 001 - 1 byte  TX address width
																							- 100 - 4 bytes TX address width				>*/

	uint8_t  RX_PW       : 6;															/*<RX payload field width (default: 100000):
																							- 000001 - 1 byte  RX payload field width
																							- 000010 - 2 bytes RX payload field width
																								•
																								•
																								•
																							- 100000 - 32 bytes RX payload field width		>*/

	uint8_t  TX_PW       : 6;															/*<TX payload field width (default: 100000):
																							- 000001 - 1 byte  TX payload field width
																							- 000010 - 2 bytes TX payload field width
																								•
																								•
																								•
																							- 100000 - 32 bytes TX payload field width		>*/

	uint8_t  UP_CLK_FREQ : 2;															/*<Output clock frequency (default: 11):
																							- 00 - 4   MHz
																							- 01 - 2   MHz
																							- 10 - 1   MHZ
																							- 11 - 500 kHz									>*/

	uint8_t  UP_CLK_EN   : 1;															/*<Output clock enable (default: 1):
																							- 0 - No external clock signal available
																							- 1 - External clock signal enabled				>*/

	uint8_t  XOF         : 3;															/*<Crystal oscillator frequency (default: 100):
																							- 000 - 4  MHz
																							- 001 - 8  MHz
																							- 010 - 12 MHz
																							- 011 - 16 MHz
																							- 100 - 20 MHz									>*/

	uint8_t  CRC_EN		 : 1;															/*<CRC Check enable (default: 1):
																							- 0 - Disable
																							- 1 - Enable									>*/

	uint8_t  CRC_MODE    : 1;															/*<CRC- mode (default: 1):
																							- 0 - 8  CRC check bit
																							- 1 - 16 CRC check bit							>*/

	void		(*GetRFConfig)(uint8_t * data);

}NRF905_RFConfigurationTypeDef;

/**
  * @brief  NRF905's object typedef
  */
typedef struct{

	NRF905_AmplifierStatusTypeDef_e status;													/*<NRF905's Amplifier Status>*/

}NRF905_AmplifierTypeDef;



/**
  * @brief  NRF905's SPI controller object typedef
  */
typedef struct{

	NRF905_SPIStatusTypeDef_e  	  status;													/*< NRF905's SPI controller status>*/

}NRF905_SPITypeDef;

/**
  * @brief  NRF905 object typedef
  */
typedef struct{
	SPI_HandleTypeDef   		  *hspi1;													/*< SPI  handle used in BMS's firmware | sending   data via NRF905>*/


	NRF905_AmplifierTypeDef       amplifier;									    		/*< NRF905's amplifier object>*/
	NRF905_SPITypeDef		      spi;														/*< NRF905's SPI object>*/
	NRF905_StatusTypeDef_e     	  status;										    		/*< NRF905's status>*/

	NRF905_RFConfigurationTypeDef nrfConfig;								        		/*< NRF905's configuration>*/

	uint32_t 					  selectedAddr;												/*< NRF905's selected address for SPI operations>*/

	void						  (*GetPayload)(uint8_t* data);								/*< Fetches and organize data to be send via RF>*/

}NRF905_TypeDef;


/**
  * @brief  BMS object type
  */
typedef struct{

	UART_HandleTypeDef  *huart1;															/*< UART handle used in BMS's firmware | logging   data via UART1>*/

	char* loggerMessage;																	/*< Error logger's message>*/

}BMS_ErrorLoggerTypeDef;

/**
  * @brief  BMS object type
  */
typedef struct{


	BMS_StatusTypeDef_e    status;															/*< Current BMS status>*/
	BMS_StatusTypeDef_e    prevStatus;														/*< Previous BMS status in case status change occurred>*/

	BMS_ErrorLoggerTypeDef errorLogger;														/*< Error logger typedef's object>*/

	CAN_BMSTypeDef 		   bmsCAN;															/*< BMS's CAN 	 custom typedef object>*/
	ADC_BMSTypeDef 		   bmsADC;															/*< BMS's ADC 	 custom typedef object>*/
	NRF905_TypeDef         bmsNRF905;														/*< BMS's NRF905 custom typedef object>*/

}BMS_TypeDef;


/* Macros ------------------------------------------------------------------------------------  */

// ADC

/*
 *  @brief ADC Channels' numbers macros definitions
 * */
#define ADC_VOLTAGE_CH 		(12)																/*<12th channel of ADC1>*/
#define ADC_CURRENT_CH 		(11)																/*<11th channel of ADC1>*/
#define ADC_TEMP_CH    		10																/*<10th channel of ADC1>*/

// STM32F105

#define BMS_VCC_SUPPLY      (87.0f)															/*<MAX DC Supply Voltage of BMS Master PCB>*/
#define VCC_SUPPLY_VOLTAGE  (3.3f)															/*<MAX DC Supply Voltage for STM32f105>*/


// NRF905

/**
  * @brief  BMS's NRF905's SPI selected commands' macros definitions
  * @note   NRF905_CMD
  */
#define NRF905_CMD_WC  		(0x00)															/*<Write configuration register>*/
#define	NRF905_CMD_WR  		(0x10)															/*<Read  configuration register>*/
#define	NRF905_CMD_WTP 		(0x20)															/*<Write TX payload>*/
#define	NRF905_CMD_RTP  	(0x21)															/*<Read  TX payload>*/
#define	NRF905_CMD_WTA  	(0x22)															/*<Write TX address>*/
#define	NRF905_CMD_RTA  	(0x23)															/*<Read  TX address>*/
#define	NRF905_CMD_RRP  	(0x24)															/*<Read  RX payload>*/
#define	NRF905_CMD_CC   	(0x80)															/*<Special command for fast setting CH_NO, HFREQ_PLL, PA_PWR in CONFIGURATION REGISTER>*/


#endif /* INC_BMS_TYPES_H_ */
