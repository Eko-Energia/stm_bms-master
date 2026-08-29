/**
  ******************************************************************************
  * @file    BMS_JK.c
  * @author  AGH Eko-Energy
  * @brief   JK BMS UART/CAN bridge implementation.
  ******************************************************************************
  */

#include "BMS_JK.h"
#include <string.h>

#define BMS_JK_FRAME_HEADER_LEN  (21U)
#define BMS_JK_FRAME_LEN_FIELD   (0x0013U)
#define BMS_JK_CMD_READ         (0x03U)
#define BMS_JK_SRC_HOST         (0x03U)
#define BMS_JK_END_MARK         (0x68U)

static const uint8_t BMS_JK_POLL_SEQUENCE[BMS_JK_POLL_CMD_COUNT] = {
    BMS_JK_CMD_CELL_VOLTAGES,
    BMS_JK_CMD_TEMPERATURES,
    BMS_JK_CMD_VOLTAGE,
    BMS_JK_CMD_CURRENT,
    BMS_JK_CMD_SOC,
    BMS_JK_CMD_CAPACITY
};

static volatile uint8_t s_bmsJkCommLock = 0U;

void BMS_JK_EnterCriticalSection(void)
{
    /* Do not globally disable interrupts here: SysTick is used by HAL_GetTick,
     * and stopping it freezes LED timing and other periodic functions.
     * Use a lightweight access guard to serialize JK UART transactions only. */
    if (s_bmsJkCommLock == 0U) {
        s_bmsJkCommLock = 1U;
    }
}

void BMS_JK_ExitCriticalSection(void)
{
    s_bmsJkCommLock = 0U;
}

static uint8_t BMS_JK_CountNonZeroFields(const BMS_JK_SnapshotTypeDef *snapshot)
{
    uint8_t count = 0U;

    if (snapshot == NULL) {
        return 0U;
    }

    if (snapshot->packVoltageMV != 0) {
        ++count;
    }
    if (snapshot->packCurrentMA != 0) {
        ++count;
    }
    if (snapshot->soc != 0U) {
        ++count;
    }
    if (snapshot->soh != 0U) {
        ++count;
    }
    if (snapshot->cellCount != 0U) {
        ++count;
    }
    for (uint8_t i = 0U; i < BMS_JK_MAX_CELL_COUNT; ++i) {
        if (snapshot->cellVoltageMV[i] != 0U) {
            ++count;
        }
    }
    if (snapshot->tempSensorCount != 0U) {
        ++count;
    }
    if (snapshot->mosTemperatureC != 0) {
        ++count;
    }
    if (snapshot->ambientTemperatureC != 0) {
        ++count;
    }
    if (snapshot->remainingCapacityMah != 0U) {
        ++count;
    }
    if (snapshot->fullCapacityMah != 0U) {
        ++count;
    }
    if (snapshot->cycleCount != 0U) {
        ++count;
    }
    if (snapshot->alarmFlags != 0U) {
        ++count;
    }
    if (snapshot->balanceActive != 0U) {
        ++count;
    }
    if (snapshot->protectionFlag != 0U) {
        ++count;
    }

    return count;
}

uint8_t BMS_JK_GetNextPollCommand(BMS_JK_HandleTypeDef *jk)
{
    uint8_t cmd;

    if (jk == NULL) {
        return BMS_JK_CMD_CELL_VOLTAGES;
    }

    if (jk->pollIndex >= BMS_JK_POLL_CMD_COUNT) {
        jk->pollIndex = 0U;
    }

    cmd = BMS_JK_POLL_SEQUENCE[jk->pollIndex];
    jk->pollIndex = (uint8_t)((jk->pollIndex + 1U) % BMS_JK_POLL_CMD_COUNT);
    return cmd;
}

uint8_t BMS_JK_IsSnapshotValidForCommand(const BMS_JK_SnapshotTypeDef *snapshot, uint8_t command)
{
    if (snapshot == NULL) {
        return 0U;
    }

    if (BMS_JK_CountNonZeroFields(snapshot) == 0U) {
        return 0U;
    }

    switch (command) {
    case BMS_JK_CMD_CELL_VOLTAGES:
        if (snapshot->cellCount == 0U) {
            return 0U;
        }
        for (uint8_t i = 0U; i < snapshot->cellCount; ++i) {
            if ((snapshot->cellVoltageMV[i] > 1000U) && (snapshot->cellVoltageMV[i] < 6000U)) {
                return 1U;
            }
        }
        return 0U;
    case BMS_JK_CMD_TEMPERATURES:
        if ((snapshot->mosTemperatureC != 0) || (snapshot->ambientTemperatureC != 0) || (snapshot->tempSensorCount != 0U)) {
            return 1U;
        }
        return 0U;
    case BMS_JK_CMD_VOLTAGE:
        return (snapshot->packVoltageMV > 1000U) && (snapshot->packVoltageMV < 700000U);
    case BMS_JK_CMD_CURRENT:
        return (snapshot->packCurrentMA != 0);
    case BMS_JK_CMD_SOC:
        return (snapshot->soc > 0U) && (snapshot->soc <= 100U);
    case BMS_JK_CMD_CAPACITY:
        return (snapshot->remainingCapacityMah != 0U) || (snapshot->fullCapacityMah != 0U);
    default:
        return BMS_JK_CountNonZeroFields(snapshot) > 0U;
    }
}

void BMS_JK_UpdateCommandState(BMS_JK_HandleTypeDef *jk, uint8_t command, uint8_t isValid)
{
    if (jk == NULL) {
        return;
    }

    jk->lastReadCommand = command;
    jk->lastReadValid = isValid;

    if (isValid != 0U) {
        jk->lastGoodCommand = command;
        jk->validationFlags &= (uint8_t)~BMS_JK_FLAG_NEEDS_RETRY;
        jk->pollIndex = (uint8_t)((jk->pollIndex + 1U) % BMS_JK_POLL_CMD_COUNT);
    } else {
        jk->validationFlags |= BMS_JK_FLAG_NEEDS_RETRY;
        jk->pollIndex = (uint8_t)((jk->pollIndex + 1U) % BMS_JK_POLL_CMD_COUNT);
    }
}

void BMS_JK_UpdateValidationFlags(BMS_JK_HandleTypeDef *jk, uint8_t sentCommand, uint8_t responseValid, uint8_t payloadZero)
{
    if (jk == NULL) {
        return;
    }

    jk->validationFlags = 0U;
    if (responseValid != 0U) {
        jk->validationFlags |= BMS_JK_FLAG_VALID_RESPONSE;
    }
    if (payloadZero != 0U) {
        jk->validationFlags |= BMS_JK_FLAG_ZERO_DATA;
    }
    if ((responseValid == 0U) || (payloadZero != 0U)) {
        jk->validationFlags |= BMS_JK_FLAG_NEEDS_RETRY;
    }

    if ((responseValid != 0U) && (payloadZero == 0U)) {
        jk->lastGoodCommand = sentCommand;
        jk->validationFlags |= BMS_JK_FLAG_USE_LAST_GOOD;
    } else {
        jk->validationFlags &= (uint8_t)~BMS_JK_FLAG_USE_LAST_GOOD;
    }

    jk->lastReadCommand = sentCommand;
    jk->lastReadValid = responseValid;
    jk->retryCount = (payloadZero != 0U) ? (uint8_t)(jk->retryCount + 1U) : 0U;
}

uint8_t BMS_JK_ShouldRetryCommand(const BMS_JK_HandleTypeDef *jk)
{
    if (jk == NULL) {
        return 0U;
    }

    return (jk->validationFlags & BMS_JK_FLAG_NEEDS_RETRY) != 0U;
}

uint8_t BMS_JK_ApplyReadoutDecision(BMS_JK_HandleTypeDef *jk,
                                    uint8_t sentCommand,
                                    uint8_t responseOk,
                                    uint8_t payloadZero,
                                    uint8_t decodeOk,
                                    uint8_t snapshotOk)
{
    uint8_t valid = 0U;

    if (jk == NULL) {
        return 0U;
    }

    if ((responseOk != 0U) && (decodeOk != 0U) && (payloadZero == 0U) && (snapshotOk != 0U)) {
        valid = 1U;
    }

    BMS_JK_UpdateValidationFlags(jk, sentCommand, responseOk, payloadZero);

    if (valid != 0U) {
        BMS_JK_UpdateCommandState(jk, sentCommand, 1U);
        jk->validationFlags |= BMS_JK_FLAG_COMMAND_LOCKED;
        return 1U;
    }

    BMS_JK_UpdateCommandState(jk, sentCommand, 0U);
    jk->validationFlags &= (uint8_t)~BMS_JK_FLAG_COMMAND_LOCKED;
    BMS_JK_ClearSnapshot(jk);
    return 0U;
}

static uint16_t BMS_JK_CalcCrc16Sum(const uint8_t *data, uint16_t len)
{
    uint32_t sum = 0U;
    for (uint16_t i = 0U; i < len; ++i) {
        sum += data[i];
    }
    return (uint16_t)(sum & 0xFFFFU);
}

uint8_t BMS_JK_ValidateResponseFrame(const uint8_t *data, uint16_t len, uint8_t expectedCommand)
{
    uint16_t crc;

    if ((data == NULL) || (len < 21U)) {
        return 0U;
    }

    if ((data[0] != 0x4EU) || (data[1] != 0x57U) || (data[2] != 0x00U) || (data[3] != 0x13U)) {
        return 0U;
    }

    if ((data[8] != BMS_JK_CMD_READ) || (data[9] != BMS_JK_SRC_HOST) || (data[10] != 0x00U)) {
        return 0U;
    }

    if ((data[11] != expectedCommand) || (data[16] != BMS_JK_END_MARK)) {
        return 0U;
    }

    crc = (uint16_t)(((uint16_t)data[19] << 8U) | (uint16_t)data[20]);
    if (BMS_JK_CalcCrc16Sum(data, 17U) != crc) {
        return 0U;
    }

    for (uint16_t i = 0U; i < len; ++i) {
        if (data[i] != 0U) {
            return 1U;
        }
    }

    return 0U;
}

static uint16_t BMS_JK_EncodeU16LE(uint8_t *dst, uint16_t value)
{
    dst[0] = (uint8_t)(value & 0xFFU);
    dst[1] = (uint8_t)((value >> 8U) & 0xFFU);
    return 2U;
}

static uint16_t BMS_JK_EncodeU32LE(uint8_t *dst, uint32_t value)
{
    dst[0] = (uint8_t)(value & 0xFFU);
    dst[1] = (uint8_t)((value >> 8U) & 0xFFU);
    dst[2] = (uint8_t)((value >> 16U) & 0xFFU);
    dst[3] = (uint8_t)((value >> 24U) & 0xFFU);
    return 4U;
}

void BMS_JK_Init(BMS_JK_HandleTypeDef *jk,
                UART_HandleTypeDef *huart,
                GPIO_TypeDef *dirPort,
                uint16_t dirPin,
                GPIO_TypeDef *rePort,
                uint16_t rePin)
{
    if (jk == NULL) {
        return;
    }

    jk->huart = huart;
    jk->rs485_dir_port = dirPort;
    jk->rs485_dir_pin = dirPin;
    jk->rs485_re_port = rePort;
    jk->rs485_re_pin = rePin;
    jk->rxLen = 0U;
    jk->txLen = 0U;
    jk->initialized = 1U;
    jk->lastGoodCommand = 0U;
    jk->lastReadCommand = 0U;
    jk->pollIndex = 0U;
    jk->lastReadValid = 0U;
    jk->validationFlags = 0U;
    jk->retryCount = 0U;

    BMS_JK_ClearSnapshot(jk);
    BMS_JK_SetRxMode(jk, 1U);
}

void BMS_JK_SetTxMode(BMS_JK_HandleTypeDef *jk, uint8_t enabled)
{
    if ((jk == NULL) || (jk->rs485_dir_port == NULL)) {
        return;
    }

    HAL_GPIO_WritePin(jk->rs485_dir_port, jk->rs485_dir_pin, enabled ? GPIO_PIN_SET : GPIO_PIN_RESET);
    if (jk->rs485_re_port != NULL) {
        HAL_GPIO_WritePin(jk->rs485_re_port, jk->rs485_re_pin, enabled ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
}

void BMS_JK_SetRxMode(BMS_JK_HandleTypeDef *jk, uint8_t enabled)
{
    if ((jk == NULL) || (jk->rs485_re_port == NULL)) {
        return;
    }

    HAL_GPIO_WritePin(jk->rs485_re_port, jk->rs485_re_pin, enabled ? GPIO_PIN_SET : GPIO_PIN_RESET);
    if (jk->rs485_dir_port != NULL) {
        HAL_GPIO_WritePin(jk->rs485_dir_port, jk->rs485_dir_pin, enabled ? GPIO_PIN_RESET : GPIO_PIN_SET);
    }
}

HAL_StatusTypeDef BMS_JK_SendRequest(BMS_JK_HandleTypeDef *jk, uint8_t command, uint32_t timeout)
{
    uint16_t crc;

    if ((jk == NULL) || (jk->huart == NULL)) {
        return HAL_ERROR;
    }

    jk->txLen = 0U;
    jk->lastReadCommand = command;
    jk->txBuffer[jk->txLen++] = 0x4EU;
    jk->txBuffer[jk->txLen++] = 0x57U;
    jk->txBuffer[jk->txLen++] = (uint8_t)((BMS_JK_FRAME_LEN_FIELD >> 8U) & 0xFFU);
    jk->txBuffer[jk->txLen++] = (uint8_t)(BMS_JK_FRAME_LEN_FIELD & 0xFFU);
    jk->txBuffer[jk->txLen++] = 0x00U;
    jk->txBuffer[jk->txLen++] = 0x00U;
    jk->txBuffer[jk->txLen++] = 0x00U;
    jk->txBuffer[jk->txLen++] = 0x00U;
    jk->txBuffer[jk->txLen++] = BMS_JK_CMD_READ;
    jk->txBuffer[jk->txLen++] = BMS_JK_SRC_HOST;
    jk->txBuffer[jk->txLen++] = 0x00U;
    jk->txBuffer[jk->txLen++] = command;
    jk->txBuffer[jk->txLen++] = 0x00U;
    jk->txBuffer[jk->txLen++] = 0x00U;
    jk->txBuffer[jk->txLen++] = 0x00U;
    jk->txBuffer[jk->txLen++] = 0x00U;
    jk->txBuffer[jk->txLen++] = BMS_JK_END_MARK;
    jk->txBuffer[jk->txLen++] = 0x00U;
    jk->txBuffer[jk->txLen++] = 0x00U;

    crc = BMS_JK_CalcCrc16Sum(jk->txBuffer, 17U);
    jk->txBuffer[jk->txLen++] = (uint8_t)((crc >> 8U) & 0xFFU);
    jk->txBuffer[jk->txLen++] = (uint8_t)(crc & 0xFFU);

    BMS_JK_EnterCriticalSection();
    BMS_JK_SetTxMode(jk, 1U);
    if (HAL_UART_Transmit(jk->huart, jk->txBuffer, jk->txLen, timeout) != HAL_OK) {
        BMS_JK_SetRxMode(jk, 1U);
        BMS_JK_ExitCriticalSection();
        return HAL_ERROR;
    }

    BMS_JK_SetRxMode(jk, 1U);
    BMS_JK_ExitCriticalSection();
    return HAL_OK;
}

HAL_StatusTypeDef BMS_JK_ReceiveResponse(BMS_JK_HandleTypeDef *jk, uint32_t timeout)
{
    if ((jk == NULL) || (jk->huart == NULL)) {
        return HAL_ERROR;
    }

    BMS_JK_EnterCriticalSection();
    jk->rxLen = 0U;
    memset(jk->rxBuffer, 0, sizeof(jk->rxBuffer));

    if (HAL_UART_Receive(jk->huart, jk->rxBuffer, BMS_JK_MAX_RX_BYTES, timeout) != HAL_OK) {
        BMS_JK_ExitCriticalSection();
        return HAL_ERROR;
    }

    jk->rxLen = (uint16_t)BMS_JK_MAX_RX_BYTES;
    BMS_JK_ExitCriticalSection();
    return HAL_OK;
}

HAL_StatusTypeDef BMS_JK_DecodeFrame(BMS_JK_HandleTypeDef *jk, const uint8_t *data, uint16_t len)
{
    uint16_t pos;
    uint16_t length;
    const uint8_t *payload;

    if ((jk == NULL) || (data == NULL) || (len < 5U)) {
        return HAL_ERROR;
    }

    /* JK BMS response logic from the verified Python analyzer.
     * Cell voltage block: 0x79 <length> <cell_idx><volt_hi><volt_lo> ...
     */
    for (pos = 0U; pos + 2U < len; ++pos) {
        if (data[pos] == BMS_JK_CMD_CELL_VOLTAGES) {
            length = (uint16_t)data[pos + 1U];
            if (pos + 2U + length <= len) {
                payload = &data[pos + 2U];
                jk->snapshot.cellCount = (length > BMS_JK_MAX_CELL_COUNT * 3U) ? BMS_JK_MAX_CELL_COUNT : (uint8_t)(length / 3U);
                for (uint8_t i = 0U; i < jk->snapshot.cellCount; ++i) {
                    uint16_t mv = (uint16_t)payload[i * 3U + 1U] << 8U;
                    mv |= payload[i * 3U + 2U];
                    jk->snapshot.cellVoltageMV[i] = mv;
                }
                jk->initialized = 1U;
                return HAL_OK;
            }
        }
    }

    /* Generic numeric field decode used by the protocol analyzer. */
    for (pos = 0U; pos + 1U < len; ++pos) {
        uint16_t raw = (uint16_t)data[pos] << 8U | (uint16_t)data[pos + 1U];
        uint16_t value = raw & 0x7FFFU;

        if (data[pos] == 0x00U && data[pos + 1U] <= 100U) {
            jk->snapshot.soc = data[pos + 1U];
            jk->initialized = 1U;
            return HAL_OK;
        }

        if ((raw >= 2000U) && (raw <= 3000U)) {
            jk->snapshot.packVoltageMV = (int32_t)(raw * 10U);
            jk->initialized = 1U;
            return HAL_OK;
        }

        if ((value <= 20000U) && ((int16_t)raw >= -20000)) {
            int16_t current = (int16_t)raw;
            jk->snapshot.packCurrentMA = (int32_t)current * 10;
            jk->initialized = 1U;
            return HAL_OK;
        }

        if ((int16_t)raw >= -40 && (int16_t)raw <= 150) {
            jk->snapshot.mosTemperatureC = (int16_t)raw;
            jk->initialized = 1U;
            return HAL_OK;
        }
    }

    return HAL_ERROR;
}

void BMS_JK_ClearSnapshot(BMS_JK_HandleTypeDef *jk)
{
    if (jk == NULL) {
        return;
    }

    memset(&jk->snapshot, 0, sizeof(jk->snapshot));
    jk->snapshot.cellCount = 0U;
    jk->snapshot.tempSensorCount = 0U;
    jk->snapshot.packVoltageMV = 0;
    jk->snapshot.packCurrentMA = 0;
    jk->snapshot.soc = 0U;
    jk->snapshot.soh = 0U;
    jk->snapshot.statusFlags = 0U;
    jk->snapshot.modeFlags = 0U;
    jk->snapshot.mosTemperatureC = 0;
    jk->snapshot.ambientTemperatureC = 0;
    jk->snapshot.cycleCount = 0U;
    jk->snapshot.remainingCapacityMah = 0U;
    jk->snapshot.fullCapacityMah = 0U;
    jk->snapshot.resistanceEstimate = 0U;
}

HAL_StatusTypeDef BMS_JK_ExportPackInfoCAN(CAN_HandleTypeDef *hcan, const BMS_JK_SnapshotTypeDef *snapshot)
{
    if ((hcan == NULL) || (snapshot == NULL)) {
        return HAL_ERROR;
    }

    CAN_TxHeaderTypeDef header;
    uint32_t mailbox;
    uint8_t payload[8] = {0};
    uint8_t *p = payload;

    header.StdId = BMS_JK_CAN_ID_PACK_INFO;
    header.IDE = CAN_ID_STD;
    header.RTR = CAN_RTR_DATA;
    header.DLC = 8U;
    header.TransmitGlobalTime = DISABLE;

    p += BMS_JK_EncodeU16LE(p, (uint16_t)(snapshot->packVoltageMV & 0xFFFFU));
    p += BMS_JK_EncodeU16LE(p, (uint16_t)(snapshot->packCurrentMA & 0xFFFFU));
    p[0] = snapshot->soc;
    p[1] = snapshot->soh;
    p[2] = snapshot->statusFlags;
    p[3] = snapshot->modeFlags;

    return HAL_CAN_AddTxMessage(hcan, &header, payload, &mailbox);
}

HAL_StatusTypeDef BMS_JK_ExportCellVoltagesCAN(CAN_HandleTypeDef *hcan, const BMS_JK_SnapshotTypeDef *snapshot)
{
    if ((hcan == NULL) || (snapshot == NULL)) {
        return HAL_ERROR;
    }

    CAN_TxHeaderTypeDef header;
    uint32_t mailbox;
    uint8_t payload[8] = {0};
    uint8_t cellIndex = 0U;

    header.StdId = BMS_JK_CAN_ID_CELL_VOLTS_1_8;
    header.IDE = CAN_ID_STD;
    header.RTR = CAN_RTR_DATA;
    header.DLC = 8U;
    header.TransmitGlobalTime = DISABLE;

    for (uint8_t i = 0U; i < 4U; ++i) {
        if (cellIndex < snapshot->cellCount) {
            uint16_t value = snapshot->cellVoltageMV[cellIndex];
            payload[i * 2U] = (uint8_t)(value & 0xFFU);
            payload[i * 2U + 1U] = (uint8_t)((value >> 8U) & 0xFFU);
            cellIndex++;
        }
    }

    return HAL_CAN_AddTxMessage(hcan, &header, payload, &mailbox);
}

HAL_StatusTypeDef BMS_JK_ExportTemperatureSummaryCAN(CAN_HandleTypeDef *hcan, const BMS_JK_SnapshotTypeDef *snapshot)
{
    if ((hcan == NULL) || (snapshot == NULL)) {
        return HAL_ERROR;
    }

    CAN_TxHeaderTypeDef header;
    uint32_t mailbox;
    uint8_t payload[8] = {0};

    header.StdId = BMS_JK_CAN_ID_TEMP_SUMMARY;
    header.IDE = CAN_ID_STD;
    header.RTR = CAN_RTR_DATA;
    header.DLC = 8U;
    header.TransmitGlobalTime = DISABLE;

    payload[0] = (uint8_t)(snapshot->cellTemperatureC[0] & 0xFF);
    payload[1] = (uint8_t)(snapshot->cellTemperatureC[1] & 0xFF);
    payload[2] = (uint8_t)(snapshot->cellTemperatureC[2] & 0xFF);
    payload[3] = (uint8_t)(snapshot->mosTemperatureC & 0xFF);
    payload[4] = (uint8_t)(snapshot->ambientTemperatureC & 0xFF);
    payload[5] = snapshot->tempSensorCount;
    payload[6] = snapshot->alarmFlags;
    payload[7] = 0U;

    return HAL_CAN_AddTxMessage(hcan, &header, payload, &mailbox);
}

HAL_StatusTypeDef BMS_JK_ExportAlarmStatusCAN(CAN_HandleTypeDef *hcan, const BMS_JK_SnapshotTypeDef *snapshot)
{
    if ((hcan == NULL) || (snapshot == NULL)) {
        return HAL_ERROR;
    }

    CAN_TxHeaderTypeDef header;
    uint32_t mailbox;
    uint8_t payload[8] = {0};

    header.StdId = BMS_JK_CAN_ID_ALARM_STATUS;
    header.IDE = CAN_ID_STD;
    header.RTR = CAN_RTR_DATA;
    header.DLC = 8U;
    header.TransmitGlobalTime = DISABLE;

    payload[0] = 0U;
    payload[1] = 0U;
    payload[2] = 0U;
    payload[3] = 0U;
    payload[4] = snapshot->alarmFlags;
    payload[5] = snapshot->balanceActive;
    payload[6] = snapshot->protectionFlag;
    payload[7] = snapshot->modeFlags;

    return HAL_CAN_AddTxMessage(hcan, &header, payload, &mailbox);
}

HAL_StatusTypeDef BMS_JK_ExportCycleStatsCAN(CAN_HandleTypeDef *hcan, const BMS_JK_SnapshotTypeDef *snapshot)
{
    if ((hcan == NULL) || (snapshot == NULL)) {
        return HAL_ERROR;
    }

    CAN_TxHeaderTypeDef header;
    uint32_t mailbox;
    uint8_t payload[8] = {0};
    uint8_t *p = payload;

    header.StdId = BMS_JK_CAN_ID_CYCLE_STATS;
    header.IDE = CAN_ID_STD;
    header.RTR = CAN_RTR_DATA;
    header.DLC = 8U;
    header.TransmitGlobalTime = DISABLE;

    p += BMS_JK_EncodeU16LE(p, snapshot->cycleCount);
    p += BMS_JK_EncodeU32LE(p, snapshot->remainingCapacityMah);
    p += BMS_JK_EncodeU32LE(p, snapshot->fullCapacityMah);
    p[0] = snapshot->resistanceEstimate;
    p[1] = 0U;

    return HAL_CAN_AddTxMessage(hcan, &header, payload, &mailbox);
}
