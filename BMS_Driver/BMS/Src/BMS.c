/**
  ******************************************************************************
  * @file      BMS.c
  * @author    Bartosz Rychlicki
  * @Title     C source files of all functions' bodies required for implementation for BMS Master's PCB board
  * @brief     Functions contain logical implementation of BMS workflow.
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
#include "BMS.h"
#include "BMS_ADC_driver.h"
#include "BMS_CAN_driver.h"
#include "BMS_PWM.h"

/* Variables ---------------------------------------------------------------------------------  */
extern uint32_t lastTick;
extern uint32_t pwmStartupStart;
static uint32_t s_ledLastTick;			/*< Last tick when status LED was toggled (module-private) >*/
/* Functions' bodies -------------------------------------------------------------------------  */

/*
	 ==============================================================================
						   ##### INIT #####
	 ==============================================================================
*/

HAL_StatusTypeDef BMS_Init(BMS_TypeDef* bms,  CAN_HandleTypeDef* bhcan1, CAN_HandleTypeDef* bhcan2, ADC_HandleTypeDef* hadc, UART_HandleTypeDef* huart, TIM_HandleTypeDef* htim){

	// reading current tick for LEDs blinkink
	s_ledLastTick = HAL_GetTick();

	// assigning handle objects
	bms->bmsADC.hadc        = hadc;
	bms->bmsCAN.bhcan1      = bhcan1;
	bms->bmsCAN.bhcan2      = bhcan2;
	bms->errorLogger.huart1 = huart;
	bms->bpwm.htim.htim 	= htim;

	// setting default status (normal) for BMS
	bms->status     = BMS_NORMAL;
	bms->prevStatus = BMS_NORMAL;

	// Reset ADC scaled TX buffer (voltage / temperature / current)
	memset(bms->bmsADC.ADC_voltTempCurr, 0 , sizeof(bms->bmsADC.ADC_voltTempCurr));

	/* Clear full CAN2 thermistor matrix [7 PCBs][9 therms] — sizeof whole array, not one byte/row */
	memset(bms->bmsCAN.CAN2_temperatureCells, 0, sizeof(bms->bmsCAN.CAN2_temperatureCells));

	/* Zero CAN1 scheduled TX list (size/txMailbox/callbacks) before EH_init consumes it */
	memset(&bms->bmsCAN.CAN1_Buff, 0, sizeof(bms->bmsCAN.CAN1_Buff));

	/*
	 * PWM MUST be brought up before any other peripheral.
	 * Relay control is safety-critical — if BMS_CAN_Init or BMS_ADC_Init later
	 * fails and we bail out of BMS_Init, the timer must already be running so
	 * the relay driver sees a live PWM (previous order left PB0 stuck at 0 V
	 * whenever CAN init failed).
	 */
	if(BMS_PWM_Init(bms, htim) != HAL_OK){
		return HAL_ERROR;
	}
	if(BMS_PWM_ChandeMode(bms, PWM_Startup) != HAL_OK){
		return HAL_ERROR;
	}
	pwmStartupStart = HAL_GetTick();

	// Launching CAN1 and CAN2
	if(BMS_CAN_Init(bms) != HAL_OK){
		return HAL_ERROR;
	}

	// Init ADC
	if(BMS_ADC_Init(bms) != HAL_OK){
		return HAL_ERROR;
	}

	// Init EH
    EH_init(&bms->beh, bms->bmsCAN.bhcan1, BMS_NODE, &(bms->bmsCAN.CAN1_Buff));

    // Setting default safestate status
    bms->safeStateStatus = 0;

    // setting default value of max stored temperarture
    bms->maxTemperature = 0.0f;

    // Setting default state of FAN
    bms->fanState = OFF;

	return HAL_OK;
}

/*
	 ==============================================================================
						   ##### MODE HANDLERS #####
	 ==============================================================================
*/

HAL_StatusTypeDef BMS_Mode_Normal(BMS_TypeDef* bms){

	// re-launching peripherals in case change of status occurred
	if(bms->prevStatus != BMS_NORMAL){

		if(BMS_Start_Peripherals(bms) != HAL_OK){
			return HAL_ERROR;
		}

		bms->prevStatus = BMS_NORMAL;
	}

	// Read ADC's channels
	if(BMS_ADC_ReadValues(bms) != HAL_OK){
		return HAL_ERROR;
	}

	// Handle received frames from CAN2
	if(BMS_CAN_HandleRxMsg(bms) != HAL_OK){
		return HAL_ERROR;
	}

	// Send Data via CAN
	CAN_HandleScheduled(bms->bmsCAN.bhcan1, &bms->bmsCAN.CAN1_Buff);

	// Handle PWM generation
	if(BMS_PWM_NormalMode(bms) != HAL_OK){
		return HAL_ERROR;
	}

	// Handling HV Sensing - if safe state and HVIL indicate equal state
	if(BMS_HVIL_Handler(bms) != HAL_OK){
		return HAL_ERROR;
	}

	// Handling FAN controller
	if(BMS_FAN_Control(bms) != HAL_OK){
		return HAL_ERROR;
	}

	return HAL_OK;
}

HAL_StatusTypeDef BMS_Mode_Error(BMS_TypeDef* bms){

	if(bms->prevStatus != BMS_Error){

		// Stopping peripherals
		if(BMS_Stop_Peripherals(bms) != HAL_OK){
			return HAL_ERROR;
		}

		// saving status
		bms->prevStatus = BMS_Error;
	}

	// Handling PWM generation in error/sleep mode
	if(BMS_PWM_SleepMode(bms) != HAL_OK){
		return HAL_ERROR;
	}

	return HAL_OK;
}

HAL_StatusTypeDef BMS_Mode_Change(BMS_TypeDef* bms, BMS_StatusTypeDef_e status){

	if(NULL == bms){
		return HAL_ERROR;
	}

	/*
	 * State-transition-only.
	 * Stop/start of peripherals is owned by BMS_Mode_Normal / BMS_Mode_Error,
	 * which trigger it exactly once on prevStatus edge.
	 * Doing it here as well caused HAL_ADC_Stop() to be called on an already
	 * stopped ADC in the next loop iteration and returned HAL_ERROR.
	 */
	bms->prevStatus = bms->status;
	bms->status     = status;

	return HAL_OK;
}


HAL_StatusTypeDef BMS_Log_Data(BMS_TypeDef* bms){


	return HAL_OK;
}

/*
	 ==============================================================================
						   ##### PERIPHERALS #####
	 ==============================================================================
*/

HAL_StatusTypeDef BMS_Start_Peripherals(BMS_TypeDef* bms){

	if(NULL == bms){
		return HAL_ERROR;
	}

/*
	 ==============================================================================
	                       ##### LAUNCHING ADC #####
	 ==============================================================================
*/

	/* Restart ADC DMA stream (ADC_Init programs channels + starts DMA). */
	if(ADC_Init(bms->bmsADC.hadc, &bms->bmsADC.cadc1, &bms->bmsADC.badc1) != HAL_OK){
		return HAL_ERROR;
	}

/*
	 ==============================================================================
						   ##### LAUNCHING CAN #####
	 ==============================================================================
*/

	/*
	 * Do NOT call BMS_CAN_Init here — filters are already programmed by BMS_Init
	 * and reprogramming them at runtime forces both controllers into FINIT,
	 * causing a brief RX outage on CAN1. Just resume from sleep and re-arm RX IRQ.
	 */
	if(HAL_CAN_IsSleepActive(bms->bmsCAN.bhcan1)){
		if(HAL_CAN_WakeUp(bms->bmsCAN.bhcan1) != HAL_OK){
			return HAL_ERROR;
		}
	}

	if(HAL_CAN_IsSleepActive(bms->bmsCAN.bhcan2)){
		if(HAL_CAN_WakeUp(bms->bmsCAN.bhcan2) != HAL_OK){
			return HAL_ERROR;
		}
	}

	/* Re-enable RX FIFO0 notification on CAN2 (deactivated by BMS_Stop_Peripherals) */
	if(HAL_CAN_ActivateNotification(bms->bmsCAN.bhcan2, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK){
		return HAL_ERROR;
	}

	return HAL_OK;
}


HAL_StatusTypeDef BMS_Stop_Peripherals(BMS_TypeDef* bms){

	if(NULL == bms){
		return HAL_ERROR;
	}

/*
	 ==============================================================================
						   ##### STOPPING ADC #####
	 ==============================================================================
*/

	/*
	 * For DMA-driven ADC we must call ONLY HAL_ADC_Stop_DMA — it stops both
	 * conversions and the DMA stream and moves the state machine to READY.
	 * Calling HAL_ADC_Stop() first would flip the state and make the following
	 * HAL_ADC_Stop_DMA() return HAL_ERROR (state != BUSY_INTERNAL).
	 */
	if(HAL_ADC_Stop_DMA(bms->bmsADC.hadc) !=  HAL_OK){
		return HAL_ERROR;
	}

/*
	 ==============================================================================
						   ##### STOPPING CAN #####
	 ==============================================================================
*/


	// Stopping CAN2 peripheral workflow for BMS in Standby or Error Mode
	if(HAL_CAN_Stop(bms->bmsCAN.bhcan2) != HAL_OK){
		return HAL_ERROR;
	}

	// Deactivating Interrupts for CAN2
	if(HAL_CAN_DeactivateNotification(bms->bmsCAN.bhcan2, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK){
		return HAL_ERROR;
	}



	return HAL_OK;
}

/*
	 ==============================================================================
						   ##### SAFETY / COOLING #####
	 ==============================================================================
*/

HAL_StatusTypeDef BMS_HVIL_Handler(BMS_TypeDef* bms){

	if(NULL == bms){
		return HAL_ERROR;
	}


#ifdef PROD

	if(HAL_GPIO_ReadPin(HVIL_GPIO_Port, HVIL_Pin) == GPIO_PIN_SET &&
		bms->safeStateStatus == SAFE_STATE_OK){



		// reporting error handler
		EH_report(&bms->beh, EH_SAFE_STATE_LEAK, ERROR_SEVERITY_SAFE_STATE);

	}

#endif

	return HAL_OK;

}

HAL_StatusTypeDef BMS_FAN_Control(BMS_TypeDef* bms){

	// Checking if correct pointer to bms object was given
	if(NULL == bms){
		return HAL_ERROR;
	}

	/*
	 * Hysteresis on latched maxTemperature (updated only after a full 7x9 unique CAN2 scan):
	 *   ON  when max >= PRE_COOLING_TEMP  (50 °C)
	 *   OFF when max <= POST_COOLING_TEMP (40 °C)
	 * Do not clear maxTemperature here — scan logic owns that value.
	 */
	if(bms->maxTemperature >= PRE_COOLING_TEMP && OFF == bms->fanState){
		bms->fanState = ON;
	}
	else if(bms->maxTemperature <= POST_COOLING_TEMP && ON == bms->fanState){
		bms->fanState = OFF;
	}

	/* Drive FAN_CONTROL GPIO from fanState */
	switch(bms->fanState){

		case ON:
			HAL_GPIO_WritePin(FAN_CONTROL_GPIO_Port, FAN_CONTROL_Pin, GPIO_PIN_SET);
			break;

		case OFF:
			HAL_GPIO_WritePin(FAN_CONTROL_GPIO_Port, FAN_CONTROL_Pin, GPIO_PIN_RESET);
			break;

		default:
			return HAL_ERROR;
	}

	return HAL_OK;
}

/*
	 ==============================================================================
						   ##### STATUS LED #####
	 ==============================================================================
*/

void BMS_Mode_LEDBlink(BMS_TypeDef* bms){

	if(NULL == bms){
		return;
	}

	/* Unsigned tick delta — wraparound safe */
	if(HAL_GetTick() - s_ledLastTick >= BMS_LED_PERIOD){

		switch(bms->status){

			// executing blinking for normal BMS's state
			case BMS_NORMAL:

				// Toggling GREEN LED
				HAL_GPIO_TogglePin(GREEN_LD_GPIO_Port, GREEN_LD_Pin);

				// Turning off RED lED
				HAL_GPIO_WritePin(RED_LD_GPIO_Port, RED_LD_Pin, GPIO_PIN_RESET);

				break;

			// executing blinking for error BMS's state
			case BMS_Error:

				// Toggling RED LED
				HAL_GPIO_TogglePin(RED_LD_GPIO_Port, RED_LD_Pin);

				// Turning off GREEN lED
				HAL_GPIO_WritePin(GREEN_LD_GPIO_Port, GREEN_LD_Pin, GPIO_PIN_RESET);

				break;
			default:
				break;
		}

		// updating tick
		s_ledLastTick = HAL_GetTick();

	}
}
