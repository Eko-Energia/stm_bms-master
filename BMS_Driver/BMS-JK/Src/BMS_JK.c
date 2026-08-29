/**
  ******************************************************************************
  * @file      BMS_JK.c
  * @brief     JK-BMS RS485 Communication and Frame Decoding Implementation
  ******************************************************************************
  */

#include "BMS_JK.h"
#include <string.h>

static const char JK_COMMANDS[BMS_JK_CMD_COUNT][42] = {
    [BMS_JK_CMD_SOC]                    ="4E57001300000000030300850000000068000001AB",
    [BMS_JK_CMD_VOLTAGE]                ="4E57001300000000030300830000000068000001A9",
    [BMS_JK_CMD_CURRENT]                ="4E57001300000000030300840000000068000001AA",
    [BMS_JK_CMD_TEMPERATURE]            ="4E57001300000000030300810000000068000001A7",
    [BMS_JK_CMD_CELL_VOLTAGES]          ="4E570013000000000303007900000000680000019F",
    [BMS_JK_CMD_CYCLES]                 ="4E57001300000000030300870000000068000001AD",
    [BMS_JK_CMD_TOTAL_STRINGS]          ="4E570013000000000303008A0000000068000001B0",
    [BMS_JK_CMD_WARNINGS]               ="4E570013000000000303008B0000000068000001B1",
    [BMS_JK_CMD_STATUS]                 ="4E570013000000000303008C0000000068000001B2",
    [BMS_JK_CMD_TOTAL_OVERVOLTAGE_PROT] ="4E570013000000000303008E0000000068000001B4",
    [BMS_JK_CMD_TOTAL_UNDERVOLTAGE_PROT]="4E570013000000000303008F0000000068000001B5",
    [BMS_JK_CMD_SINGLE_OVERVOLTAGE_PROT]="4E57001300000000030300900000000068000001B6",
    [BMS_JK_CMD_SINGLE_OVERVOLTAGE_RECOV] = "4E57001300000000030300910000000068000001B7",
    [BMS_JK_CMD_DIFF_VOLTAGE_PROT]       ="4E57001300000000030300930000000068000001B9",
    [BMS_JK_CMD_DISCHARGE_OVERCURRENT_PROT] = "4E57001300000000030300940000000068000001BA",
    [BMS_JK_CMD_CHARGE_OVERCURRENT_PROT] ="4E57001300000000030300960000000068000001BC",
    [BMS_JK_CMD_BATTERY_TYPE]           ="4E57001300000000030300AF0000000068000001D5",
    [BMS_JK_CMD_ACTUAL_CAPACITY]        ="4E57001300000000030300B90000000068000001DF",
    [BMS_JK_CMD_DEVICE_ID]              ="4E57001300000000030300B40000000068000001DA",
    [BMS_JK_CMD_PRODUCTION_DATE]        ="4E57001300000000030300B50000000068000001DB",
    [BMS_JK_CMD_SYSTEM_TIME]            ="4E57001300000000030300B60000000068000001DC",
    [BMS_JK_CMD_SOFTWARE_VERSION]       ="4E57001300000000030300B70000000068000001DD"
};

static void BMS_JK_SetTransceiverMode(BMS_JK_HandleTypeDef *jk, BMS_JK_TrxMode_t mode)
{
    if (jk != NULL) {
        if (mode == BMS_JK_TRX_MODE_TX) {
            // ST3485EB: DE = HIGH, RE = HIGH (Driver enabled, receiver disabled)
            if (jk->de_port != NULL) {
                HAL_GPIO_WritePin(jk->de_port, jk->de_pin, GPIO_PIN_SET);
            }
            if (jk->re_port != NULL) {
                HAL_GPIO_WritePin(jk->re_port, jk->re_pin, GPIO_PIN_SET);
            }
        } else {
            // ST3485EB: DE = LOW, RE = LOW (Driver disabled, receiver enabled)
            if (jk->de_port != NULL) {
                HAL_GPIO_WritePin(jk->de_port, jk->de_pin, GPIO_PIN_RESET);
            }
            if (jk->re_port != NULL) {
                HAL_GPIO_WritePin(jk->re_port, jk->re_pin, GPIO_PIN_RESET);
            }
        }
    }
}

HAL_StatusTypeDef BMS_JK_Init(BMS_JK_HandleTypeDef *jk, UART_HandleTypeDef *huart, 
                              GPIO_TypeDef *de_port, uint16_t de_pin, 
                              GPIO_TypeDef *re_port, uint16_t re_pin)
{
    if (jk == NULL || huart == NULL) {
        return HAL_ERROR;
    }
    
    memset(jk, 0, sizeof(BMS_JK_HandleTypeDef));
    jk->huart = huart;
    jk->de_port = de_port;
    jk->de_pin = de_pin;
    jk->re_port = re_port;
    jk->re_pin = re_pin;
    
    BMS_JK_ClearSnapshot(jk);
    BMS_JK_SetTransceiverMode(jk, BMS_JK_TRX_MODE_RX);
    return HAL_OK;
}

void BMS_JK_ClearSnapshot(BMS_JK_HandleTypeDef *jk)
{
    if (jk != NULL) {
        memset(&jk->snapshot, 0, sizeof(BMS_JK_SnapshotTypeDef));
        jk->initialized = 0U;
    }
}

HAL_StatusTypeDef BMS_JK_Update(BMS_JK_HandleTypeDef *jk, uint32_t timeout)
{
    uint8_t cmd = BMS_JK_GetNextPollCommand(jk);
    if (BMS_JK_SendRequest(jk, cmd, timeout) == HAL_OK) {
        if (BMS_JK_ReceiveResponse(jk, timeout) == HAL_OK) {
            return BMS_JK_DecodeFrame(jk, jk->rxBuffer, jk->rxLen);
        }
    }
    return HAL_ERROR;
}

HAL_StatusTypeDef BMS_JK_DecodeFrame(BMS_JK_HandleTypeDef *jk, const uint8_t *data, uint16_t len)
{
    uint16_t i;
    int data_parsed = 0;

    if ((jk == NULL) || (data == NULL) || (len < 3)) {
        return HAL_ERROR;
    }

    // Scan the response buffer for register tags, matching the working Python parsing logic
    for (i = 0; i < len; i++) {
        uint8_t tag = data[i];

        if (tag == 0x79U) { // Cell voltages block
            if (i + 1 >= len) break;
            uint8_t cell_data_len = data[i + 1];
            uint8_t num_cells = cell_data_len / 3U;
            if (num_cells > 0U && num_cells <= BMS_JK_MAX_CELL_COUNT && (i + 2 + cell_data_len <= len)) {
                jk->snapshot.cellCount = num_cells;
                int32_t total_mv = 0;
                for (uint8_t c = 0U; c < num_cells; c++) {
                    uint16_t mv = ((uint16_t)data[i + 2 + (c * 3U) + 1] << 8U) | data[i + 2 + (c * 3U) + 2U];
                    jk->snapshot.cellVoltageMV[c] = mv;
                    total_mv += (int32_t)mv;
                }
                if (jk->snapshot.packVoltageMV == 0) {
                    jk->snapshot.packVoltageMV = total_mv;
                }
                data_parsed = 1;
            }
            i += (uint16_t)(1U + cell_data_len);
        }
        else if (tag == 0x83U) { // Pack voltage
            if (i + 2 >= len) break;
            uint16_t v_raw = ((uint16_t)data[i + 1] << 8U) | data[i + 2];
            jk->snapshot.packVoltageMV = (int32_t)v_raw * 10; // Convert 0.01V units to mV
            data_parsed = 1;
        }
        else if (tag == 0x84U) { // Pack current
            if (i + 2 >= len) break;
            int16_t cur_raw = (int16_t)(((uint16_t)data[i + 1] << 8U) | data[i + 2]);
            jk->snapshot.packCurrentMA = (int32_t)cur_raw * 10;
            data_parsed = 1;
        }
        else if (tag == 0x85U) { // SOC
            if (i + 1 >= len) break;
            jk->snapshot.soc = data[i + 1];
            data_parsed = 1;
        }
        else if (tag == 0x81U) { // Temperature
            if (i + 2 >= len) break;
            int16_t temp_raw = (int16_t)(((uint16_t)data[i + 1] << 8U) | data[i + 2]);
            if (temp_raw > 100) {
                jk->snapshot.mosTemperatureC = temp_raw - 100;
            } else {
                jk->snapshot.mosTemperatureC = -temp_raw;
            }
            data_parsed = 1;
        }
        else if (tag == 0x87U) { // Cycles
            if (i + 2 >= len) break;
            uint16_t cycles_raw = ((uint16_t)data[i + 1] << 8U) | data[i + 2];
            jk->snapshot.cycles = cycles_raw;
            data_parsed = 1;
        }
    }

    if (data_parsed != 0) {
        jk->initialized = 1U; 
        return HAL_OK;
    }

    return HAL_ERROR;
}

HAL_StatusTypeDef BMS_JK_SendRequest(BMS_JK_HandleTypeDef *jk, uint8_t commandId, uint32_t timeout)
{
    HAL_StatusTypeDef status;

    if ((jk == NULL) || (jk->huart == NULL) || (commandId >= BMS_JK_CMD_COUNT)) {
        return HAL_ERROR;
    }

    jk->lastReadCommand = commandId;

    // Enable driver mode for ST3485EB
    BMS_JK_SetTransceiverMode(jk, BMS_JK_TRX_MODE_TX); 

    status = HAL_UART_Transmit(jk->huart, (uint8_t*)JK_COMMANDS[commandId], BMS_JK_CMD_LEN, timeout);
    
    // Wait for Transmission Complete (TC) shift register flag
    uint32_t tickstart = HAL_GetTick();
    while (__HAL_UART_GET_FLAG(jk->huart, UART_FLAG_TC) == RESET) {
        if ((HAL_GetTick() - tickstart) > timeout) {
            break;
        }
    }

    // Brief guard time for RS485 turnaround before switching back to RX
    for(volatile int d = 0; d < 400; d++);

    return status;
}

HAL_StatusTypeDef BMS_JK_PackAllCommandsBuffer(uint8_t *pDestBuffer, uint16_t *pTotalLen)
{
    if (pDestBuffer == NULL || pTotalLen == NULL) {
        return HAL_ERROR;
    }

    uint16_t offset = 0;
    for (uint8_t i = 0; i < BMS_JK_CMD_COUNT; i++) {
        memcpy(&pDestBuffer[offset], JK_COMMANDS[i], BMS_JK_CMD_LEN);
        offset += BMS_JK_CMD_LEN;
    }
    
    *pTotalLen = offset;
    return HAL_OK;
}

HAL_StatusTypeDef BMS_JK_SendAllCommandsGate(BMS_JK_HandleTypeDef *jk, uint32_t timeout)
{
    uint8_t packedBuffer[BMS_JK_CMD_COUNT * BMS_JK_CMD_LEN];
    uint16_t totalLen = 0;

    if ((jk == NULL) || (jk->huart == NULL)) {
        return HAL_ERROR;
    }

    if (BMS_JK_PackAllCommandsBuffer(packedBuffer, &totalLen) != HAL_OK) {
        return HAL_ERROR;
    }

    // Enable driver mode for ST3485EB transceiver
    BMS_JK_SetTransceiverMode(jk, BMS_JK_TRX_MODE_TX);

    // Transmit the entire packed command block in a single transmission gate
    if (HAL_UART_Transmit(jk->huart, packedBuffer, totalLen, timeout) != HAL_OK) {
        BMS_JK_SetTransceiverMode(jk, BMS_JK_TRX_MODE_RX);
        return HAL_ERROR;
    }
    
    // Wait for Transmission Complete (TC) flag to clear the hardware shift register
    uint32_t tickstart = HAL_GetTick();
    while (__HAL_UART_GET_FLAG(jk->huart, UART_FLAG_TC) == RESET) {
        if ((HAL_GetTick() - tickstart) > timeout) {
            break;
        }
    }

    // Brief guard delay before switching transceiver back to receive mode
    for (volatile int d = 0; d < 400; d++);

    return HAL_OK;
}

uint8_t BMS_JK_GetNextPollCommand(BMS_JK_HandleTypeDef *jk)
{
    static uint8_t poll_index = 0;
    static const uint8_t poll_sequence[] = {
        BMS_JK_CMD_VOLTAGE,
        BMS_JK_CMD_CURRENT,
        BMS_JK_CMD_SOC,
        BMS_JK_CMD_TEMPERATURE,
        BMS_JK_CMD_CELL_VOLTAGES,
        BMS_JK_CMD_CYCLES
    };

    if (jk != NULL) {
        poll_index = (poll_index + 1) % (sizeof(poll_sequence) / sizeof(poll_sequence[0]));
        jk->lastReadCommand = poll_sequence[poll_index];
        return jk->lastReadCommand;
    }
    return BMS_JK_CMD_VOLTAGE;
}

HAL_StatusTypeDef BMS_JK_ReceiveResponse(BMS_JK_HandleTypeDef *jk, uint32_t timeout)
{
    uint32_t tickstart = HAL_GetTick();
    uint16_t rxIndex = 0U;
    uint8_t tempByte;
    uint32_t lastByteTick;

    if ((jk == NULL) || (jk->huart == NULL)) {
        return HAL_ERROR;
    }

    BMS_JK_SetTransceiverMode(jk, BMS_JK_TRX_MODE_RX); 
    
    memset(jk->rxBuffer, 0, sizeof(jk->rxBuffer));
    jk->rxLen = 0U;
    lastByteTick = HAL_GetTick();

    while ((HAL_GetTick() - tickstart) < timeout) {
        // Dynamically clear overrun, noise, and framing errors during reception
        if (__HAL_UART_GET_FLAG(jk->huart, UART_FLAG_ORE) != RESET ||
            __HAL_UART_GET_FLAG(jk->huart, UART_FLAG_NE)  != RESET ||
            __HAL_UART_GET_FLAG(jk->huart, UART_FLAG_FE)  != RESET) {
            __HAL_UART_CLEAR_OREFLAG(jk->huart);
            __HAL_UART_CLEAR_NEFLAG(jk->huart);
            __HAL_UART_CLEAR_FEFLAG(jk->huart);
        }

        if (HAL_UART_Receive(jk->huart, &tempByte, 1, 5) == HAL_OK) {
            if (rxIndex < BMS_JK_MAX_RX_BYTES) {
                jk->rxBuffer[rxIndex++] = tempByte;
            }
            lastByteTick = HAL_GetTick();
        } else {
            // 15ms of bus silence signals the end of the packet frame
            if (rxIndex > 0U && (HAL_GetTick() - lastByteTick) > 15U) {
                break;
            }
        }
    }

    // Optional robustness filter: Strip out local RS485 loopback echo if present
    if (rxIndex >= BMS_JK_CMD_LEN) {
        if (memcmp(jk->rxBuffer, JK_COMMANDS[jk->lastReadCommand], BMS_JK_CMD_LEN) == 0) {
            uint16_t remaining_len = rxIndex - BMS_JK_CMD_LEN;
            if (remaining_len > 0) {
                memmove(jk->rxBuffer, &jk->rxBuffer[BMS_JK_CMD_LEN], remaining_len);
            }
            rxIndex = remaining_len;
        }
    }

    jk->rxLen = rxIndex;
    return (jk->rxLen > 0U) ? HAL_OK : HAL_ERROR;
}

uint8_t BMS_JK_ValidateResponseFrame(const uint8_t *buffer, uint16_t len, uint8_t commandId)
{
    (void)commandId;
    if (buffer == NULL || len < 5U) {
        return 0U;
    }
    
    for (uint16_t i = 0; i < len - 1U; i++) {
        if (buffer[i] == 0x4EU && buffer[i+1] == 0x57U) {
            return 1U;
        }
    }
    return 0U;
}

uint8_t BMS_JK_IsSnapshotValidForCommand(const BMS_JK_SnapshotTypeDef *snapshot, uint8_t commandId)
{
    (void)snapshot;
    (void)commandId;
    return 1U; 
}

void BMS_JK_ApplyReadoutDecision(BMS_JK_HandleTypeDef *jk, uint8_t commandId, uint8_t responseValid, uint8_t payloadZero, uint8_t decodeOk, uint8_t snapshotOk)
{
    (void)commandId;
    (void)payloadZero;
    (void)snapshotOk;
    if (jk == NULL) {
        return;
    }

    if ((responseValid != 0U) || (decodeOk != 0U)) {
        jk->initialized = 1U; 
    }
}
