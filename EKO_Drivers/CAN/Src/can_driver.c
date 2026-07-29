/**
  * @file can_driver.c
  * @brief CAN bus driver for PERLA
  * @author AGH EKO-ENERGIA
  * @author Kacper Lasota
  */

/*
 *
 *
 * Error handling both on bus and generic error messages
 * Filter configuration
 * Received messages handling
 *
 */
#include "can_driver.h"

/* Include error handler if available */
#if __has_include("error_handler.h")
#include "error_handler.h"
#define ERROR_HANDLER_AVAILABLE (1)
#else
#define ERROR_HANDLER_AVAILABLE (0)
#endif

/**
 * @brief Initialize CAN peripheral
 *
 * @param hcanPtr   Pointer to CAN handle
 */
void CAN_Init(CAN_HandleTypeDef *hcanPtr)
{
	if(CAN2 == hcanPtr->Instance){
		if (HAL_CAN_ActivateNotification(hcanPtr, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
		{
			Error_Handler();
		}

		CAN_FilterTypeDef filterConfig = {0};

		/*
		 * CAN2 RX filters (SlaveStartFilterBank = 14 on F105 connectivity line):
		 *   Bank 14 → SAFE_STATE StdId = 1 (exact match)
		 *   Bank 15 → thermistor StdIds 0x200..0x27F (mask); app still bounds-checks pcb/therm
		 * StdId is placed in FilterIdHigh[15:5] for 32-bit scale filters.
		 */

		/* ----- Filter bank 14: SAFE_STATE_ID (1) exact ----- */
		filterConfig.FilterBank = 14;
		filterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
		filterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
		filterConfig.FilterIdHigh = (1U << 5);				/* StdId = 1 */
		filterConfig.FilterIdLow = 0x0000;
		filterConfig.FilterMaskIdHigh = (0x7FFU << 5);		/* all 11 StdId bits must match */
		filterConfig.FilterMaskIdLow = 0x0000;
		filterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
		filterConfig.FilterActivation = ENABLE;
		filterConfig.SlaveStartFilterBank = 14;

		if (HAL_CAN_ConfigFilter(hcanPtr, &filterConfig) != HAL_OK)
		{
			Error_Handler();
		}

		/* ----- Filter bank 15: thermistor StdIds (software bounds-checks pcb/therm).
		 * IDs are decimal 211..279 (BASE 200 + pcb*10 + therm), NOT hex 0x200.
		 * Mask don't-care on StdId; SAFE_STATE is exact-matched on bank 14.
		 */
		filterConfig.FilterBank = 15;
		filterConfig.FilterIdHigh = 0x0000;
		filterConfig.FilterIdLow = 0x0000;
		filterConfig.FilterMaskIdHigh = 0x0000;		/* accept any StdId; app filters map */
		filterConfig.FilterMaskIdLow = 0x0000;

		if (HAL_CAN_ConfigFilter(hcanPtr, &filterConfig) != HAL_OK)
		{
			Error_Handler();
		}
	}

	if (HAL_CAN_Start(hcanPtr) != HAL_OK)
	{
		Error_Handler();
	}
}

/**
 * @brief Add new message to the periodic buffer
 *
 * @param msg      Pointer to the message to add
 * @param buffer   Pointer to the buffer that holds messages
 * @retval HAL_StatusTypeDef   State of the operation
 */
HAL_StatusTypeDef CAN_AddScheduledMsg(const struct CAN_scheduledMsg *msg, struct CAN_scheduledMsgList *buffer)
{
	// basic error checking
	if (buffer->size >= CAN_MAX_MSG)
	{
		Error_Handler();
	}
	if (msg->periodMs == 0)
	{
		Error_Handler();
	}

	struct CAN_scheduledMsg tempMsg = *msg;
	tempMsg.lastTick = HAL_GetTick();

	// check if id already exists in the buffer
	for (uint8_t i = 0; i < buffer->size; i++)
	{
		if ((buffer->list[i].header.IDE == CAN_ID_STD && buffer->list[i].header.StdId == tempMsg.header.StdId) ||
			(buffer->list[i].header.IDE == CAN_ID_EXT && buffer->list[i].header.ExtId == tempMsg.header.ExtId))
		{
			return HAL_ERROR;
		}
	}

	buffer->list[buffer->size] = tempMsg;
	buffer->size++;
	return HAL_OK;
}

/**
 * @brief Remove message from the periodic buffer
 *
 * @param id       ID of the message to remove
 * @param buffer   Pointer to the buffer that holds messages
 * @retval HAL_StatusTypeDef   State of the operation
 */
HAL_StatusTypeDef CAN_RemoveScheduledMsg(uint32_t id, struct CAN_scheduledMsgList *buffer)
{
	for (uint8_t i = 0; i < buffer->size; i++)
	{
		if ((buffer->list[i].header.IDE == CAN_ID_STD && buffer->list[i].header.StdId == id) ||
			(buffer->list[i].header.IDE == CAN_ID_EXT && buffer->list[i].header.ExtId == id))
		{
			while (i + 1 < buffer->size)
			{
				buffer->list[i] = buffer->list[i + 1];
				i++;
			}
			buffer->size--;
			return HAL_OK;
		}
	}

	return HAL_ERROR;
}

/**
 * @brief Process all scheduled CAN messages (call in main loop)
 *
 * @param hcanPtr      Pointer to CAN handle
 * @param scheduler    Pointer to the message scheduler
 */
void CAN_HandleScheduled(CAN_HandleTypeDef *hcanPtr, struct CAN_scheduledMsgList *scheduler)
{
	if (hcanPtr == NULL || scheduler == NULL)
	{
		return;
	}

	uint32_t currentTick = HAL_GetTick();
	for (uint8_t i = 0; i < scheduler->size; i++)
	{
		struct CAN_scheduledMsg *msg = &scheduler->list[i];
		if (currentTick > msg->lastTick + msg->periodMs)
		{
			uint8_t data[CAN_MAX_DLC];
			// Initialize data to 0 to be safe
			for (uint8_t k = 0; k < CAN_MAX_DLC; k++)
			{
				data[k] = 0;
			}
			
			if (msg->getData != NULL)
			{
				msg->getData(data, msg->context);
			}
			
			if (HAL_CAN_AddTxMessage(hcanPtr, &msg->header, data, &scheduler->txMailbox) != HAL_OK)
			{
				return;
			}

			msg->lastTick = HAL_GetTick();
		}
	}
}
