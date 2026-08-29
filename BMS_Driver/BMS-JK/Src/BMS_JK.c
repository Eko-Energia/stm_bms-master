/**
  ******************************************************************************
  * @file      BMS_JK.c
  * @brief     JK-BMS RS485 Communication and Frame Decoding Implementation
  ******************************************************************************
  */

#include "BMS_JK.h"
#include <string.h>

static const uint8_t JK_COMMANDS[BMS_JK_CMD_COUNT][BMS_JK_CMD_LEN] = {
    [BMS_JK_CMD_SOC]                    = {0x4E, 0x57, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x00, 0x85, 0x00, 0x00, 0x00, 0x00, 0x68, 0x00, 0x00, 0x01, 0xAB},
    [BMS_JK_CMD_VOLTAGE]                = {0x4E, 0x57, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x00, 0x83, 0x00, 0x00, 0x00, 0x00, 0x68, 0x00, 0x00, 0x01, 0xA9},
    [BMS_JK_CMD_CURRENT]                = {0x4E, 0x57, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x00, 0x84, 0x00, 0x00, 0x00, 0x00, 0x68, 0x00, 0x00, 0x01, 0xAA},
    [BMS_JK_CMD_TEMPERATURE]            = {0x4E, 0x57, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x00, 0x81, 0x00, 0x00, 0x00, 0x00, 0x68, 0x00, 0x00, 0x01, 0xA7},
    [BMS_JK_CMD_CELL_VOLTAGES]          = {0x4E, 0x57, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x00, 0x79, 0x00, 0x00, 0x00, 0x00, 0x68, 0x00, 0x00, 0x01, 0x9F},
    [BMS_JK_CMD_CYCLES]                 = {0x4E, 0x57, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x00, 0x87, 0x00, 0x00, 0x00, 0x00, 0x68, 0x00, 0x00, 0x01, 0xAD},
    [BMS_JK_CMD_TOTAL_STRINGS]          = {0x4E, 0x57, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x00, 0x8A, 0x00, 0x00, 0x00, 0x00, 0x68, 0x00, 0x00, 0x01, 0xB0},
    [BMS_JK_CMD_WARNINGS]               = {0x4E, 0x57, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x00, 0x8B, 0x00, 0x00, 0x00, 0x00, 0x68, 0x00, 0x00, 0x01, 0xB1},
    [BMS_JK_CMD_STATUS]                 = {0x4E, 0x57, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x00, 0x8C, 0x00, 0x00, 0x00, 0x00, 0x68, 0x00, 0x00, 0x01, 0xB2},
    [BMS_JK_CMD_TOTAL_OVERVOLTAGE_PROT] = {0x4E, 0x57, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x00, 0x8E, 0x00, 0x00, 0x00, 0x00, 0x68, 0x00, 0x00, 0x01, 0xB4},
    [BMS_JK_CMD_TOTAL_UNDERVOLTAGE_PROT]= {0x4E, 0x57, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x00, 0x8F, 0x00, 0x00, 0x00, 0x00, 0x68, 0x00, 0x00, 0x01, 0xB5},
    [BMS_JK_CMD_SINGLE_OVERVOLTAGE_PROT]= {0x4E, 0x57, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x00, 0x90, 0x00, 0x00, 0x00, 0x00, 0x68, 0x00, 0x00, 0x01, 0xB6},
    [BMS_JK_CMD_SINGLE_OVERVOLTAGE_RECOV] = {0x4E, 0x57, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x00, 0x91, 0x00, 0x00, 0x00, 0x00, 0x68, 0x00, 0x00, 0x01, 0xB7},
    [BMS_JK_CMD_DIFF_VOLTAGE_PROT]       = {0x4E, 0x57, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x00, 0x93, 0x00, 0x00, 0x00, 0x00, 0x68, 0x00, 0x00, 0x01, 0xB9},
    [BMS_JK_CMD_DISCHARGE_OVERCURRENT_PROT] = {0x4E, 0x57, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x00, 0x94, 0x00, 0x00, 0x00, 0x00, 0x68, 0x00, 0x00, 0x01, 0xBA},
    [BMS_JK_CMD_CHARGE_OVERCURRENT_PROT] = {0x4E, 0x57, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x00, 0x96, 0x00, 0x00, 0x00, 0x00, 0x68, 0x00, 0x00, 0x01, 0xBC},
    [BMS_JK_CMD_BATTERY_TYPE]           = {0x4E, 0x57, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x00, 0xAF, 0x00, 0x00, 0x00, 0x00, 0x68, 0x00, 0x00, 0x01, 0xD5},
    [BMS_JK_CMD_ACTUAL_CAPACITY]        = {0x4E, 0x57, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x00, 0xB9, 0x00, 0x00, 0x00, 0x00, 0x68, 0x00, 0x00, 0x01, 0xDF},
    [BMS_JK_CMD_DEVICE_ID]              = {0x4E, 0x57, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x00, 0xB4, 0x00, 0x00, 0x00, 0x00, 0x68, 0x00, 0x00, 0x01, 0xDA},
    [BMS_JK_CMD_PRODUCTION_DATE]        = {0x4E, 0x57, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x00, 0xB5, 0x00, 0x00, 0x00, 0x00, 0x68, 0x00, 0x00, 0x01, 0xDB},
    [BMS_JK_CMD_SYSTEM_TIME]            = {0x4E, 0x57, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x00, 0xB6, 0x00, 0x00, 0x00, 0x00, 0x68, 0x00, 0x00, 0x01, 0xDC},
    [BMS_JK_CMD_SOFTWARE_VERSION]       = {0x4E, 0x57, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x00, 0xB7, 0x00, 0x00, 0x00, 0x00, 0x68, 0x00, 0x00, 0x01, 0xDD}
};

static void BMS_JK_SetTransceiverMode(BMS_JK_HandleTypeDef *jk, BMS_JK_TrxMode_t mode)
{
    if (jk != NULL) {
        if (mode == BMS_JK_TRX_MODE_TX) {
            if (jk->de_port != NULL) {
                HAL_GPIO_WritePin(jk->de_port, jk->de_pin, GPIO_PIN_SET);
            }
            if (jk->re_port != NULL) {
                HAL_GPIO_WritePin(jk->re_port, jk->re_pin, GPIO_PIN_SET);
            }
        } else {
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

HAL_StatusTypeDef BMS_JK_Normal(BMS_JK_HandleTypeDef *jk, uint32_t timeout)
{

    // 1. Prepare command
    uint8_t cmd = BMS_JK_GetNextPollCommand(jk);
    uint8_t txBuffer[BMS_JK_CMD_LEN];
    memcpy(txBuffer, JK_COMMANDS[cmd], BMS_JK_CMD_LEN);
    memset(jk->rxBuffer, 0, BMS_JK_MAX_RX_BYTES);
    jk->rxLen = 0U;
    jk->lastReadCommand = cmd;

    // 2. Disable global interrupts for tight RS-485 turna round timing
    __disable_irq();

    // 3. Switch to Transmit Mode (DE=1, RE=1)
    BMS_JK_SetTransceiverMode(jk, BMS_JK_TRX_MODE_TX);

    // 4. Transmit command frame via HAL
    if (HAL_UART_Transmit(jk->huart, txBuffer, BMS_JK_CMD_LEN, timeout) != HAL_OK) {
        BMS_JK_SetTransceiverMode(jk, BMS_JK_TRX_MODE_RX);
        __enable_irq();
        return HAL_ERROR;
    }

    // Wait until the final stop bit completely leaves the UART shift register
    while (__HAL_UART_GET_FLAG(jk->huart, UART_FLAG_TC) == RESET);
    __HAL_UART_CLEAR_FLAG(jk->huart, UART_FLAG_TC);

    // Brief bus turnaround guard delay (~20 microseconds)
    for (volatile int d = 0; d < 50; d++);

    // 5. Switch to Receive Mode (DE=0, RE=0)
    BMS_JK_SetTransceiverMode(jk, BMS_JK_TRX_MODE_RX);

    // Clear any stale UART error flags
    __HAL_UART_CLEAR_OREFLAG(jk->huart);
    __HAL_UART_CLEAR_NEFLAG(jk->huart);
    __HAL_UART_CLEAR_FEFLAG(jk->huart);

    // 6. Direct register-level reception loop (bypasses HAL pointer bugs and tick freezes)
    uint16_t rxIndex = 0U;
    uint32_t loop_limit = timeout * 15000UL;

    while (rxIndex < BMS_JK_MAX_RX_BYTES && loop_limit > 0U) {
        loop_limit--;
        if (__HAL_UART_GET_FLAG(jk->huart, UART_FLAG_RXNE) != RESET) {
            jk->rxBuffer[rxIndex++] = (uint8_t)(jk->huart->Instance->DR & 0xFF);
            loop_limit = 50000U; // Reset timeout window upon capturing a valid byte
        }
    }
    jk->rxLen = rxIndex;

    // 7. Re-enable interrupts
    __enable_irq();

    // 8. Frame decoding and command echo stripping
    if (jk->rxLen >= BMS_JK_CMD_LEN) {
        if (jk->rxBuffer[0] == 0x4EU && jk->rxBuffer[1] == 0x57U &&
            jk->rxBuffer[11] == JK_COMMANDS[jk->lastReadCommand][11]) {
            uint16_t remaining_len = jk->rxLen - BMS_JK_CMD_LEN;
            if (remaining_len > 0U) {
                memmove(jk->rxBuffer, &jk->rxBuffer[BMS_JK_CMD_LEN], remaining_len);
            }
            jk->rxLen = remaining_len;
        }
    }

    return BMS_JK_DecodeFrame(jk, jk->rxBuffer, jk->rxLen);
}

HAL_StatusTypeDef BMS_JK_DecodeFrame(BMS_JK_HandleTypeDef *jk, const uint8_t *data, uint16_t len)
{
    uint16_t i;
    int data_parsed = 0;

    if ((jk == NULL) || (data == NULL) || (len < 3)) {
        return HAL_ERROR;
    }

    for (i = 0; i < len; i++) {
        uint8_t tag = data[i];

        if (tag == 0x79U) { 
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
        else if (tag == 0x83U) { 
            if (i + 2 >= len) break;
            uint16_t v_raw = ((uint16_t)data[i + 1] << 8U) | data[i + 2];
            jk->snapshot.packVoltageMV = (int32_t)v_raw * 10;
            data_parsed = 1;
        }
        else if (tag == 0x84U) { 
            if (i + 2 >= len) break;
            int16_t cur_raw = (int16_t)(((uint16_t)data[i + 1] << 8U) | data[i + 2]);
            jk->snapshot.packCurrentMA = (int32_t)cur_raw * 10;
            data_parsed = 1;
        }
        else if (tag == 0x85U) { 
            if (i + 1 >= len) break;
            jk->snapshot.soc = data[i + 1];
            data_parsed = 1;
        }
        else if (tag == 0x81U) { 
            if (i + 2 >= len) break;
            int16_t temp_raw = (int16_t)(((uint16_t)data[i + 1] << 8U) | data[i + 2]);
            if (temp_raw > 100) {
                jk->snapshot.mosTemperatureC = temp_raw - 100;
            } else {
                jk->snapshot.mosTemperatureC = -temp_raw;
            }
            data_parsed = 1;
        }
        else if (tag == 0x87U) { 
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
        uint8_t cmd = poll_sequence[poll_index];
        poll_index = (poll_index + 1) % (sizeof(poll_sequence) / sizeof(poll_sequence[0]));
        jk->lastReadCommand = cmd;
        return cmd;
    }
    return BMS_JK_CMD_VOLTAGE;
}
