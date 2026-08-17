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

extern HAL_StatusTypeDef HAL_CAN_Init(CAN_HandleTypeDef *hcan);

/* Include error handler if available */
#if __has_include("error_handler.h")
#include "error_handler.h"
#define ERROR_HANDLER_AVAILABLE (1)
#else
#define ERROR_HANDLER_AVAILABLE (0)
#endif

/* F105 connectivity: 28 shared banks, CAN2SB=14 → banks 0-13 CAN1, 14-27 CAN2 */
#define CAN2_SLAVE_START_FILTER_BANK	(14U)
#define CAN2_FILTER_BANK_SAFE_STATE	(14U)
#define CAN2_FILTER_BANK_THERM_LO	(15U)
#define CAN2_FILTER_BANK_THERM_HI	(16U)

/* 32-bit scale: StdId[10:0] lives in FilterIdHigh[15:5] */
#define CAN_STDID_TO_FILTER_HIGH(id)	((uint16_t)((uint32_t)(id) << 5))

static void CAN_ApplyStdIdMaskFilter(CAN_HandleTypeDef *hcanPtr, uint32_t bank,
				     uint32_t stdId, uint32_t stdMask)
{
	CAN_FilterTypeDef filterConfig = {0};

	filterConfig.FilterBank = bank;
	filterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	filterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
	filterConfig.FilterIdHigh = CAN_STDID_TO_FILTER_HIGH(stdId);
	filterConfig.FilterIdLow = 0x0000;
	filterConfig.FilterMaskIdHigh = CAN_STDID_TO_FILTER_HIGH(stdMask);
	filterConfig.FilterMaskIdLow = 0x0000;
	filterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
	filterConfig.FilterActivation = ENABLE;
	filterConfig.SlaveStartFilterBank = CAN2_SLAVE_START_FILTER_BANK;

	if (HAL_CAN_ConfigFilter(hcanPtr, &filterConfig) != HAL_OK)
	{
		Error_Handler();
	}
}

/**
 * @brief Program CAN2 slave RX filters (CAN1 FMR / filter banks).
 *
 * Call while both instances are still READY. HAL_CAN_ConfigFilter always
 * sets CAN1 FMR.FINIT; doing that after CAN1 has left init can prevent
 * CAN2 from clearing INAK in HAL_CAN_Start().
 */
void CAN_ConfigRxFilters(CAN_HandleTypeDef *hcanPtr)
{
	if (hcanPtr == NULL || hcanPtr->Instance != CAN2)
	{
		return;
	}

	/*
	 * Bank 14: SAFE_STATE StdId = 1 exact.
	 * Banks 15-16: thermistor decimal 211..279 (0x0D3..0x117), NOT hex 0x200.
	 *   15 → 0x0C0..0x0FF (covers 211..255)
	 *   16 → 0x100..0x11F (covers 256..279; 280..287 dropped in software)
	 */
	CAN_ApplyStdIdMaskFilter(hcanPtr, CAN2_FILTER_BANK_SAFE_STATE, 1U, 0x7FFU);
	CAN_ApplyStdIdMaskFilter(hcanPtr, CAN2_FILTER_BANK_THERM_LO, 0x0C0U, 0x7C0U);
	CAN_ApplyStdIdMaskFilter(hcanPtr, CAN2_FILTER_BANK_THERM_HI, 0x100U, 0x7E0U);
}

/**
 * @brief Start CAN peripheral
 *
 * @param hcanPtr   Pointer to CAN handle
 */
void CAN_Init(CAN_HandleTypeDef *hcanPtr)
{
	if (hcanPtr == NULL)
	{
		Error_Handler();
		return;
	}

	/* Second BMS_CAN_Init (error recovery) must not HAL_CAN_Start a LISTENING handle */
	if (hcanPtr->State != HAL_CAN_STATE_LISTENING)
	{
		if (HAL_CAN_Start(hcanPtr) != HAL_OK)
		{
			Error_Handler();
		}
	}

	if (hcanPtr->Instance == CAN2)
	{
		if (HAL_CAN_ActivateNotification(hcanPtr, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
		{
			Error_Handler();
		}
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
