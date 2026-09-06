/* Includes ----------------------------------------------------------------------------------  */
#include "BMS.h"
#include "BMS_ADC_driver.h"
#include "BMS_CAN_driver.h"
#include "BMS_PWM.h"
#include "usart.h"
#include "BMS_JK.h"

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

HAL_StatusTypeDef BMS_Init(BMS_TypeDef* bms,  CAN_HandleTypeDef* bhcan1, ADC_HandleTypeDef* hadc, UART_HandleTypeDef* huart, TIM_HandleTypeDef* htim){

	// reading current tick for LEDs blinking
	s_ledLastTick = HAL_GetTick();

	// assigning handle objects
	bms->bmsADC.hadc        = hadc;
	bms->bmsCAN.bhcan1      = bhcan1;
	bms->errorLogger.huart1 = huart;
	bms->bpwm.htim.htim 	= htim;

	// setting default status (normal) for BMS
	bms->status     = BMS_NORMAL;
	bms->prevStatus = BMS_NORMAL;

	// Reset ADC scaled TX buffer (voltage / temperature / current)
	for(uint8_t i = 0U; i < 3U; ++i){
		bms->bmsADC.ADC_voltTempCurr[i] = 0;
	}

	/* Clear thermistor matrix [7 PCBs][9 therms] */
	memset(bms->bmsCAN.CAN2_temperatureCells, 0, sizeof(bms->bmsCAN.CAN2_temperatureCells));

	/* Zero CAN1 scheduled TX list */
	memset(&bms->bmsCAN.CAN1_Buff, 0, sizeof(bms->bmsCAN.CAN1_Buff));

	/*
	 * JK RS485 MUST be initialized before PWM/CAN/ADC.
	 * If a later peripheral fails, main still enters BMS_Error — JK poll
	 * must already have a valid huart or TransmitRequest is a no-op.
	 * UART handle from main is USART1 (huart1).
	 */
	BMS_RS485_Init(bms);
	if(huart != NULL){
		(void)BMS_JK_Init(&bms->bmsJK, huart, RS_DIR_GPIO_Port, RS_DIR_Pin,
		                  RE_DIR_GPIO_Port, RE_DIR_Pin);
	}

	/*
	 * PWM MUST be brought up before any other peripheral.
	 */
	if(BMS_PWM_Init(bms, htim) != HAL_OK){
		return HAL_ERROR;
	}
	if(BMS_PWM_ChandeMode(bms, PWM_Startup) != HAL_OK){
		return HAL_ERROR;
	}
	pwmStartupStart = HAL_GetTick();

	// Launching CAN1 (TX scheduled + RX thermistor frames)
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

    // setting default value of max stored temperature
    bms->maxTemperature = 0.0f;

    // Setting default state of FAN
    bms->fanState = OFF;

	return HAL_OK;
}

void BMS_RS485_Init(BMS_TypeDef* bms){
	if(bms == NULL){
		return;
	}
	/* Default half-duplex listen: DE=0, /RE=0 */
	HAL_GPIO_WritePin(RS_DIR_GPIO_Port, RS_DIR_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(RE_DIR_GPIO_Port, RE_DIR_Pin, GPIO_PIN_RESET);
}

/*
	 ==============================================================================
						   ##### MODE HANDLERS #####
	 ==============================================================================
*/

// Inside BMS_Mode_Normal in BMS.c:
HAL_StatusTypeDef BMS_Mode_Normal(BMS_TypeDef* bms){

	// re-launching peripherals in case change of status occurred
	if(bms->prevStatus != BMS_NORMAL){

		if(BMS_Start_Peripherals(bms) != HAL_OK){
			return HAL_ERROR;
		}

		bms->prevStatus = BMS_NORMAL;
	}

	/* JK polled from main() every loop (incl. error mode). */

	// Read ADC's channels
	if(BMS_ADC_ReadValues(bms) != HAL_OK){
		return HAL_ERROR;
	}

	// Handle received frames from CAN1
	if(BMS_CAN_HandleRxMsg(bms) != HAL_OK){
		return HAL_ERROR;
	}

	// Send Data via CAN
	CAN_HandleScheduled(bms->bmsCAN.bhcan1, &bms->bmsCAN.CAN1_Buff);

	// Handle PWM generation
	if(BMS_PWM_NormalMode(bms) != HAL_OK){
		return HAL_ERROR;
	}

	// Handling HV Sensing
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

	/* JK RX/TX is invoked from main() every loop (not here). */

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

	if(ADC_Init(bms->bmsADC.hadc, &bms->bmsADC.cadc1, &bms->bmsADC.badc1) != HAL_OK){
		return HAL_ERROR;
	}

	if(HAL_CAN_IsSleepActive(bms->bmsCAN.bhcan1)){
		if(HAL_CAN_WakeUp(bms->bmsCAN.bhcan1) != HAL_OK){
			return HAL_ERROR;
		}
	}

	if(HAL_CAN_ActivateNotification(bms->bmsCAN.bhcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK){
		return HAL_ERROR;
	}

	return HAL_OK;
}

HAL_StatusTypeDef BMS_Stop_Peripherals(BMS_TypeDef* bms){

	if(NULL == bms){
		return HAL_ERROR;
	}

	if(HAL_ADC_Stop_DMA(bms->bmsADC.hadc) !=  HAL_OK){
		return HAL_ERROR;
	}

	if(HAL_CAN_Stop(bms->bmsCAN.bhcan1) != HAL_OK){
		return HAL_ERROR;
	}

	if(HAL_CAN_DeactivateNotification(bms->bmsCAN.bhcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK){
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
		EH_report(&bms->beh, EH_SAFE_STATE_LEAK, ERROR_SEVERITY_SAFE_STATE);
	}
#endif

	return HAL_OK;
}

HAL_StatusTypeDef BMS_FAN_Control(BMS_TypeDef* bms){

	if(NULL == bms){
		return HAL_ERROR;
	}

	if(bms->maxTemperature >= PRE_COOLING_TEMP && OFF == bms->fanState){
		bms->fanState = ON;
	}
	else if(bms->maxTemperature <= POST_COOLING_TEMP && ON == bms->fanState){
		bms->fanState = OFF;
	}

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

	if(HAL_GetTick() - s_ledLastTick >= BMS_LED_PERIOD){

		switch(bms->status){
			case BMS_NORMAL:
				HAL_GPIO_TogglePin(GREEN_LD_GPIO_Port, GREEN_LD_Pin);
				HAL_GPIO_WritePin(RED_LD_GPIO_Port, RED_LD_Pin, GPIO_PIN_RESET);
				break;
			case BMS_Error:
				HAL_GPIO_TogglePin(RED_LD_GPIO_Port, RED_LD_Pin);
				HAL_GPIO_WritePin(GREEN_LD_GPIO_Port, GREEN_LD_Pin, GPIO_PIN_RESET);
				break;
			default:
				break;
		}

		s_ledLastTick = HAL_GetTick();
	}
}
