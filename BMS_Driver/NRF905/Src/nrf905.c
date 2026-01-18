/**
  ******************************************************************************
  * @file      nrf905.c
  * @author    Bartosz Rychlicki
  * @author    AGH Eko-Energia
  *
  * @Title     C source files of all functions' bodies required for implementation for BMS Master's NRF905 module
  * @brief     Functions contain logical implementation for providing efficient radio communication on 868MHz.
  *
  ******************************************************************************
  * @attention Error codes are called when exact incorrect use of function is made
  *
  * Copyright (c) 2026 AGH Eko-Energy.
  * All rights reserved.
  *
  ******************************************************************************
  */

/* Includes ----------------------------------------------------------------------------------  */
#include "BMS_Types.h"

/* Variables ----------------------------------------------------------------------------------  */
extern BMS_TypeDef bms;

/* Functions' bodies -------------------------------------------------------------------------  */
// NRF905 power modes


/**
  * @brief  Function that provides NRF905 enters given mode
  * @param  BMS_TypeDef* bms - pointer to BMS object of type BMS_TypeDef
  * @param  NRF905_StatusTypeDef_e status - status enumeration object for NRF905 module
  * @retval HAL_StatusTypeDef
*/
HAL_StatusTypeDef BMS_NRF905_ChangeMode(BMS_TypeDef* bms, NRF905_StatusTypeDef_e status){

	// checking if correct status was given
	if(status != BMS_NRF905_ON && status != BMS_NRF905_OFF){
		return HAL_ERROR;
	}

	// changing status, stored in BMS' NRF905 object field to the given one
	bms->bmsNRF905.status = status;

	// providing changing NRF905 mode by electrical operations
	HAL_GPIO_WritePin(NRF905_CSN_GPIO_Port, NRF905_CSN_Pin, (status == BMS_NRF905_ON) ? GPIO_PIN_RESET : GPIO_PIN_SET);

	return HAL_OK;
}


// NRF905 operations
void NRF905_Init(void);

void NRF905_RF_Config(void);

void NRF905_TX_Payload(uint8_t* data);

void NRF905_Tx_Address(uint8_t* addr);

void NRF905_Send(uint32_t Id, uint8_t* data, uint8_t DLC);

/*
 * RF function, which formats and fetch data to configure NRF905
*/
void NRF905_GetConfigData(uint8_t* data){

	// setting byte0 with 8 bits of CH_NO
	data[0] = ((bms.bmsNRF905.nrfConfig.CH_NO)     & 0xF);

	// filling byte1 with data in order:  bit[7:6] not used, AUTO_RETRAN, RX_RED_PWR, PA_PWR[1:0], HFREQ_PLL, CH_NO[8]

	data[1] =  (bms.bmsNRF905.nrfConfig.AUTO_RETRAN << 5) |
			   (bms.bmsNRF905.nrfConfig.RX_RED_PWR  << 4) |
			   (bms.bmsNRF905.nrfConfig.PA_PWR      << 2) |
			   (bms.bmsNRF905.nrfConfig.HFREQ_PLL   << 1) |
			   (bms.bmsNRF905.nrfConfig.CH_NO 	 	>> 8);

	// filling byte2 with data in order: bit[7] not used, TX_AFW[2:0] , bit[3] not used, RX_AFW[2:0]
	data[2] =  (bms.bmsNRF905.nrfConfig.TX_AFW      << 4) |
			   (bms.bmsNRF905.nrfConfig.RX_AFW   		);

	// filling byte3 with data in order: bit[7:6] not used, RX_PW[5:0]
	data[3] =  bms.bmsNRF905.nrfConfig.RX_PW;

	// filling byte4 with data in order: bit[7:6] not used, TX_PW[5:0]
	data[4] =  bms.bmsNRF905.nrfConfig.TX_PW;

	// filling byte5 with 1st (MSB) byte of RX_ADDREESS
	data[5] = (bms.bmsNRF905.nrfConfig.RX_ADDRESS   << 24);

	// filling byte6 with 2nd byte of RX_ADDREESS
	data[6] = (bms.bmsNRF905.nrfConfig.RX_ADDRESS 	<< 16);

	// filling byte7 with 3rd byte of RX_ADDREESS
	data[7] = (bms.bmsNRF905.nrfConfig.RX_ADDRESS 	<<  8);

	// filling byte8 with 4th (LSB) byte of RX_ADDREESS
	data[8] =  bms.bmsNRF905.nrfConfig.RX_ADDRESS;

	// filling byte9 with data in order: CRC_MODE,CRC_EN, XOF[2:0], UP_CLK_EN, UP_CLK_FREQ[1:0]
	data[9] = (bms.bmsNRF905.nrfConfig.CRC_MODE	    <<  7) |
			  (bms.bmsNRF905.nrfConfig.CRC_EN	    <<  6) |
			  (bms.bmsNRF905.nrfConfig.XOF			<<  3) |
			  (bms.bmsNRF905.nrfConfig.UP_CLK_EN	<<  2) |
			  (-bms.bmsNRF905.nrfConfig.UP_CLK_FREQ	     );

}

// NRF905's SPI operations

HAL_StatusTypeDef BMS_NRF905_SPI_ChangeCMD(BMS_TypeDef* bms, NRF905_SPICommandTypeDef command){

	// security check, if correct command was given
	if(command != NRF905_CMD_WC    &&
	   command != NRF905_CMD_WR    &&
	   command != 	NRF905_CMD_WTP &&
	   command != NRF905_CMD_RTP   &&
	   command != NRF905_CMD_WTA   &&
	   command != NRF905_CMD_RTA   &&
	   command != NRF905_CMD_RRP   &&
	   command != NRF905_CMD_CC)
	{
		return HAL_ERROR;
	}

	// assigning given command to BMS's NRF905's SPI controller
	bms->bmsNRF905.spi.selectedCommand = command;


	return HAL_OK;
}

/*
  * @brief  Function that provides NRF905's SPI controller enters given mode
  * @param  BMS_TypeDef* bms - pointer to BMS object of type BMS_TypeDef
  * @param  NRF905_SPIStatusTypeDef_e status - status enumeration object for NRF905's SPI controller
  * @retval HAL_StatusTypeDef
  *
  * @note   Logic of function presented below was implemented accorning to NRF905 datasheet (and below)
  * ﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎
  * ┋ PWR_UP ┋ TRX_CE ┋ TX_EN ┋            OPERATING MODE      ┋
  * ┋┉┉┉┉┉┉┉┉┉┉┉┉┉┋┉┉┉┉┉┉┉┉┉┉┉┉┋┉┉┉┉┉┉┉┉┉┉┉┋┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉ ┋
  * ┋   0    ┋   X    ┋   X   ┋ Power down and SPI programming ┋
  * ┋   1    ┋   0    ┋   X   ┋ Standby and SPI programming    ┋
  * ┋   1    ┋   X    ┋   0   ┋ Read data from RX register     ┋
  * ┋   1    ┋   1    ┋   0   ┋ Radio Enabled - ShockBurst™ RX ┋
  * ┋   1    ┋   1    ┋   1   ┋ Radio Enabled - ShockBurst™ TX ┋
  * ﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎
*/

HAL_StatusTypeDef BMS_NRF905_SPI_ChangeMode(BMS_TypeDef* bms, NRF905_SPIStatusTypeDef_e status){

	// checking if correct status was given
	if(status != BMS_NRF905_SPI_PWRDOWN       &&
	   status != BMS_NRF905_SPI_STANDBY 	  &&
	   status != BMS_NRF905_SPI_READRXREG 	  &&
	   status != BMS_NRF905_SPI_SHOCKBURST_RX &&
	   status != BMS_NRF905_SPI_SHOCKBURST_TX)
	{
		return HAL_ERROR;
	}

	// changing status in BMS's nrf905 field
	bms->bmsNRF905.spi.status = status;

	/*
	 * Providing changing NRF905 mode by electrical operations
	*/

	// setting 0V on PWR_UP pin if given status is BMS_NRF905_SPI_PWRDOWN, otherwise set 3V3
	HAL_GPIO_WritePin(PWR_UP_GPIO_Port, PWR_UP_Pin, (status == BMS_NRF905_SPI_PWRDOWN)? GPIO_PIN_RESET : GPIO_PIN_SET);

	// setting 0V on TRX_CE pin if given status is BMS_NRF905_SPI_STANDBY, otherwise set 3V3
	HAL_GPIO_WritePin(TRX_CE_GPIO_Port, TRX_CE_Pin, (status == BMS_NRF905_SPI_STANDBY) ? GPIO_PIN_RESET : GPIO_PIN_SET);

	// setting 3V3 on TX_EN pin if given status is BMS_NRF905_SPI_SHOCKBURST_TX, otherwise set 0V
	HAL_GPIO_WritePin(TX_EN_GPIO_Port, TX_EN_Pin, (status == BMS_NRF905_SPI_SHOCKBURST_TX) ? GPIO_PIN_SET : GPIO_PIN_RESET);


	return HAL_OK;
}



/**
  * @brief  Function that provides sending data and receiving response from NRF905 module
  * @param  BMS_TypeDef* bms  - pointer to BMS object of type BMS_TypeDef, with SPI handle
  * @param  uint8_t* txByte   - pointer to address of one byte to send it to NRF905
  * @param  uint8_t* rxByte   - pointer to address of received response on sent byte from NRF905
  * @retval HAL_StatusTypeDef - status of communication with NRF905 via SPI, and status of proceeded operation
*/
HAL_StatusTypeDef NRF905_SPI_TransferReceive(BMS_TypeDef* bms, uint8_t* txByte, uint8_t* rxByte){

	// Sending and receiving one byte of data
	if(HAL_SPI_TransmitReceive(&bms->hspi1, txByte, rxByte, 1, HAL_MAX_DELAY) != HAL_OK){
		return HAL_ERROR;
	}

	return HAL_OK;
}

HAL_StatusTypeDef NRF905_WriteReg(uint8_t cmd, uint8_t* data, uint8_t len);

void NRF905_ReadReg(uint8_t cmd, uint8_t* data, uint8_t len);

// NRF905's amplifier power modes
/*
 * Logic for below function was implemented according to table from MAX2233 datasheet (and presented below)
 * ﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎
 * ┋     MODE     ┋ D1 ┋ D0 ┋ OUTPUT POWER  ┋
 * ┋┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┋┉┉┉┉┉┉┋┉┉┉┉┉┉┋┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉ ┋
 * ┋ Shutdown     ┋ 0  ┋ 0  ┋ < -35dBm      ┋
 * ┋ Low Power    ┋ 0  ┋ 1  ┋ + 6.5dBm      ┋
 * ┋ Medium Power ┋ 1  ┋ 0  ┋ + 15dBm       ┋
 * ┋ High Power   ┋ 1  ┋ 1  ┋ + 24dBm       ┋
 * ﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎﹎
*/


/**
  * @brief  Function that provides amplifier enters given mode
  * @param  BMS_TypeDef* bms - pointer to BMS object of type BMS_TypeDef
  * @param  NRF905_AmplifierStatusTypeDef_e status - status enumeration object for MAX2233 amplifier
  * @retval HAL_StatusTypeDef
*/
HAL_StatusTypeDef BMS_NRF905_Amplifier_ChangeMode(BMS_TypeDef* bms, NRF905_AmplifierStatusTypeDef_e status){

	// checking if correct mode was given
	if(status != BMS_NRF905_AMPLIFIER_SHUTDOWN  &&
	   status != BMS_NRF905_AMPLIFIER_LOWPWR    &&
	   status != BMS_NRF905_AMPLIFIER_MEDIUMPWR &&
	   status != BMS_NRF905_AMPLIFIER_HIGHPWR   )
	{
		return HAL_ERROR;
	}

	// changing status in BMS's nrf905 field
	bms->bmsNRF905.amplifier.status = status;


	// set 0V on D0 pin if MAX2233's mode is shutdown or medium PWR, otherwise set 3V3
	HAL_GPIO_WritePin(D0_GPIO_Port, D0_Pin, (status == BMS_NRF905_AMPLIFIER_SHUTDOWN || status ==  BMS_NRF905_AMPLIFIER_MEDIUMPWR) ? GPIO_PIN_RESET : GPIO_PIN_SET);

	// set 0V on D1 pin if MAX2233's mode is shutdown or low PWR, otherwise set 3V3
	HAL_GPIO_WritePin(D1_GPIO_Port, D1_Pin, (status == BMS_NRF905_AMPLIFIER_SHUTDOWN || status ==  BMS_NRF905_AMPLIFIER_LOWPWR)    ? GPIO_PIN_RESET : GPIO_PIN_SET);

	return HAL_OK;
}















