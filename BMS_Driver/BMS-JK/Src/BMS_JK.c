/**
  ******************************************************************************
  * @file    BMS_JK.c
  * @brief   Direct C port of bms_jk_sender_receiver.py
  *
  * Python:
  *   ser.reset_input_buffer()
  *   ser.write(cmd)          # 21 bytes
  *   time.sleep(0.25)
  *   response = ser.read(ser.in_waiting)
  *   parse tags...
  *
  * RS485: DE high only while shifting TX; drop DE immediately after TC.
  * /RE stays enabled (RO driven). Local TX echo drained during TX only —
  * never flush/drain after turnaround (that swallowed JK SOF / reply).
  *
  * RX: store every RXNE byte for BMS_JK_REPLY_WAIT_MS. FE/NE counted but
  * bytes kept when DR has data. Decode aligns on 4E57; raw kept if no SOF.
  * HSI-only clock — do not enable HSE.
  ******************************************************************************
  */

#include "BMS_JK.h"
#include "main.h"
#include <string.h>

static const uint8_t s_jkCmds[BMS_JK_CMD_COUNT][BMS_JK_CMD_LEN] = {
    /*  0 SOC                     */ {0x4E,0x57,0x00,0x13,0x00,0x00,0x00,0x00,0x03,0x03,0x00,0x85,0x00,0x00,0x00,0x00,0x68,0x00,0x00,0x01,0xAB},
    /*  1 Voltage                 */ {0x4E,0x57,0x00,0x13,0x00,0x00,0x00,0x00,0x03,0x03,0x00,0x83,0x00,0x00,0x00,0x00,0x68,0x00,0x00,0x01,0xA9},
    /*  2 Current                 */ {0x4E,0x57,0x00,0x13,0x00,0x00,0x00,0x00,0x03,0x03,0x00,0x84,0x00,0x00,0x00,0x00,0x68,0x00,0x00,0x01,0xAA},
    /*  3 Temperature             */ {0x4E,0x57,0x00,0x13,0x00,0x00,0x00,0x00,0x03,0x03,0x00,0x81,0x00,0x00,0x00,0x00,0x68,0x00,0x00,0x01,0xA7},
    /*  4 CellVoltages            */ {0x4E,0x57,0x00,0x13,0x00,0x00,0x00,0x00,0x03,0x03,0x00,0x79,0x00,0x00,0x00,0x00,0x68,0x00,0x00,0x01,0x9F},
    /*  5 Cycles                  */ {0x4E,0x57,0x00,0x13,0x00,0x00,0x00,0x00,0x03,0x03,0x00,0x87,0x00,0x00,0x00,0x00,0x68,0x00,0x00,0x01,0xAD},
    /*  6 TotalStrings            */ {0x4E,0x57,0x00,0x13,0x00,0x00,0x00,0x00,0x03,0x03,0x00,0x8A,0x00,0x00,0x00,0x00,0x68,0x00,0x00,0x01,0xB0},
    /*  7 Warnings                */ {0x4E,0x57,0x00,0x13,0x00,0x00,0x00,0x00,0x03,0x03,0x00,0x8B,0x00,0x00,0x00,0x00,0x68,0x00,0x00,0x01,0xB1},
    /*  8 Status                  */ {0x4E,0x57,0x00,0x13,0x00,0x00,0x00,0x00,0x03,0x03,0x00,0x8C,0x00,0x00,0x00,0x00,0x68,0x00,0x00,0x01,0xB2},
    /*  9 TotalOvervoltageProt    */ {0x4E,0x57,0x00,0x13,0x00,0x00,0x00,0x00,0x03,0x03,0x00,0x8E,0x00,0x00,0x00,0x00,0x68,0x00,0x00,0x01,0xB4},
    /* 10 TotalUndervoltageProt   */ {0x4E,0x57,0x00,0x13,0x00,0x00,0x00,0x00,0x03,0x03,0x00,0x8F,0x00,0x00,0x00,0x00,0x68,0x00,0x00,0x01,0xB5},
    /* 11 SingleOvervoltageProt   */ {0x4E,0x57,0x00,0x13,0x00,0x00,0x00,0x00,0x03,0x03,0x00,0x90,0x00,0x00,0x00,0x00,0x68,0x00,0x00,0x01,0xB6},
    /* 12 SingleOvervoltageRecov  */ {0x4E,0x57,0x00,0x13,0x00,0x00,0x00,0x00,0x03,0x03,0x00,0x91,0x00,0x00,0x00,0x00,0x68,0x00,0x00,0x01,0xB7},
    /* 13 DiffVoltageProt         */ {0x4E,0x57,0x00,0x13,0x00,0x00,0x00,0x00,0x03,0x03,0x00,0x93,0x00,0x00,0x00,0x00,0x68,0x00,0x00,0x01,0xB9},
    /* 14 DischargeOvercurrentProt*/ {0x4E,0x57,0x00,0x13,0x00,0x00,0x00,0x00,0x03,0x03,0x00,0x94,0x00,0x00,0x00,0x00,0x68,0x00,0x00,0x01,0xBA},
    /* 15 ChargeOvercurrentProt   */ {0x4E,0x57,0x00,0x13,0x00,0x00,0x00,0x00,0x03,0x03,0x00,0x96,0x00,0x00,0x00,0x00,0x68,0x00,0x00,0x01,0xBC},
    /* 16 BatteryType             */ {0x4E,0x57,0x00,0x13,0x00,0x00,0x00,0x00,0x03,0x03,0x00,0xAF,0x00,0x00,0x00,0x00,0x68,0x00,0x00,0x01,0xD5},
    /* 17 ActualCapacity          */ {0x4E,0x57,0x00,0x13,0x00,0x00,0x00,0x00,0x03,0x03,0x00,0xB9,0x00,0x00,0x00,0x00,0x68,0x00,0x00,0x01,0xDF},
    /* 18 DeviceID                */ {0x4E,0x57,0x00,0x13,0x00,0x00,0x00,0x00,0x03,0x03,0x00,0xB4,0x00,0x00,0x00,0x00,0x68,0x00,0x00,0x01,0xDA},
    /* 19 ProductionDate          */ {0x4E,0x57,0x00,0x13,0x00,0x00,0x00,0x00,0x03,0x03,0x00,0xB5,0x00,0x00,0x00,0x00,0x68,0x00,0x00,0x01,0xDB},
    /* 20 SystemTime              */ {0x4E,0x57,0x00,0x13,0x00,0x00,0x00,0x00,0x03,0x03,0x00,0xB6,0x00,0x00,0x00,0x00,0x68,0x00,0x00,0x01,0xDC},
    /* 21 SoftwareVersion         */ {0x4E,0x57,0x00,0x13,0x00,0x00,0x00,0x00,0x03,0x03,0x00,0xB7,0x00,0x00,0x00,0x00,0x68,0x00,0x00,0x01,0xDD},
};

void BMS_JK_SET_TX(BMS_JK_HandleTypeDef *jk)
{
    (void)jk;
    /* /RE stays LOW — RO remains driven */
    HAL_GPIO_WritePin(RE_DIR_GPIO_Port, RE_DIR_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(RS_DIR_GPIO_Port, RS_DIR_Pin, GPIO_PIN_SET);
}

void BMS_JK_SET_RX(BMS_JK_HandleTypeDef *jk)
{
    (void)jk;
    /* MAX485 RX: DE low + /RE low */
    HAL_GPIO_WritePin(RS_DIR_GPIO_Port, RS_DIR_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(RE_DIR_GPIO_Port, RE_DIR_Pin, GPIO_PIN_RESET);
}

static void BMS_JK_SampleDirPins(BMS_JK_HandleTypeDef *jk)
{
    jk->dePin = (HAL_GPIO_ReadPin(RS_DIR_GPIO_Port, RS_DIR_Pin) == GPIO_PIN_SET) ? 1U : 0U;
    jk->rePin = (HAL_GPIO_ReadPin(RE_DIR_GPIO_Port, RE_DIR_Pin) == GPIO_PIN_SET) ? 1U : 0U;
}

static void BMS_JK_FlushRx(UART_HandleTypeDef *huart)
{
    volatile uint32_t tmp;
    while (__HAL_UART_GET_FLAG(huart, UART_FLAG_RXNE) != RESET) {
        tmp = huart->Instance->DR;
    }
    tmp = huart->Instance->SR;
    tmp = huart->Instance->DR;
    (void)tmp;
}

/**
 * TX one frame; drop local RS485 echo during shift only (keeps DR free).
 * After last byte: wait TC without discarding RX — JK reply may start then.
 */
static HAL_StatusTypeDef BMS_JK_TransmitDropEcho(UART_HandleTypeDef *huart,
                                                 const uint8_t *data, uint16_t len)
{
    uint16_t i;
    uint32_t guard;

    for (i = 0U; i < len; i++) {
        guard = 0U;
        while (__HAL_UART_GET_FLAG(huart, UART_FLAG_TXE) == RESET) {
            if (__HAL_UART_GET_FLAG(huart, UART_FLAG_RXNE) != RESET) {
                (void)huart->Instance->DR; /* drop echo of prior byte */
            }
            if (++guard > 500000U) {
                return HAL_TIMEOUT;
            }
        }
        huart->Instance->DR = data[i];

        /* One echo byte for this TX byte — then move on (do not spin forever) */
        guard = 0U;
        while (guard++ < 50000U) {
            if (__HAL_UART_GET_FLAG(huart, UART_FLAG_RXNE) != RESET) {
                (void)huart->Instance->DR;
                break;
            }
        }
    }

    /* Wait TC — do NOT drop RXNE here (would eat early JK SOF) */
    guard = 0U;
    while (__HAL_UART_GET_FLAG(huart, UART_FLAG_TC) == RESET) {
        if (++guard > 500000U) {
            return HAL_TIMEOUT;
        }
    }

    return HAL_OK;
}

/** Pull one USART byte into *out. FE/NE counted; byte kept when RXNE/ORE set. */
static uint8_t BMS_JK_PollOneByte(BMS_JK_HandleTypeDef *jk, uint8_t *out)
{
    uint32_t sr = jk->huart->Instance->SR;

    if ((sr & (USART_SR_RXNE | USART_SR_ORE)) != 0U) {
        uint8_t b = (uint8_t)(jk->huart->Instance->DR & 0xFFU);
        if ((sr & USART_SR_ORE) != 0U) {
            jk->oreCount++;
        }
        if ((sr & (USART_SR_FE | USART_SR_NE)) != 0U) {
            jk->feCount++;
        }
        *out = b;
        return 1U;
    }

    /* Error flag without RXNE — clear by reading DR (nothing to store) */
    if ((sr & (USART_SR_FE | USART_SR_NE)) != 0U) {
        (void)jk->huart->Instance->DR;
        jk->feCount++;
    }
    return 0U;
}

static void BMS_JK_PublishRxHead(BMS_JK_HandleTypeDef *jk, uint16_t n)
{
    uint16_t i;
    jk->rxLen = n;
    jk->lastRxLen = n;
    memset(jk->rxHead, 0, sizeof(jk->rxHead));
    for (i = 0U; (i < 16U) && (i < n); i++) {
        jk->rxHead[i] = jk->rxBuf[i];
    }
}

HAL_StatusTypeDef BMS_JK_Init(BMS_JK_HandleTypeDef *jk, UART_HandleTypeDef *huart, 
                              GPIO_TypeDef *de_port, uint16_t de_pin, 
                              GPIO_TypeDef *re_port, uint16_t re_pin)
{
    (void)de_port;
    (void)de_pin;
    (void)re_port;
    (void)re_pin;

    if ((jk == NULL) || (huart == NULL)) {
        return HAL_ERROR;
    }
    
    memset(jk, 0, sizeof(*jk));
    jk->huart = huart;

    SET_BIT(huart->Instance->CR1, (USART_CR1_UE | USART_CR1_TE | USART_CR1_RE));
    huart->gState  = HAL_UART_STATE_READY;
    huart->RxState = HAL_UART_STATE_READY;
    __HAL_UNLOCK(huart);

    BMS_JK_FlushRx(huart);
    BMS_JK_SET_RX(jk);
    BMS_JK_SampleDirPins(jk);
    return HAL_OK;
}

/* ser.reset_input_buffer(); ser.write(cmd) */
HAL_StatusTypeDef BMS_JK_SendRequest(BMS_JK_HandleTypeDef *jk, uint8_t cmdIndex)
{
    if ((jk == NULL) || (jk->huart == NULL) || (cmdIndex >= BMS_JK_CMD_COUNT)) {
        return HAL_ERROR;
    }

    /* Like Python reset_input_buffer — only BEFORE write */
    BMS_JK_FlushRx(jk->huart);
    BMS_JK_SET_TX(jk);
    HAL_Delay(1);

    SET_BIT(jk->huart->Instance->CR1, (USART_CR1_UE | USART_CR1_TE | USART_CR1_RE));
    jk->huart->gState  = HAL_UART_STATE_READY;
    jk->huart->RxState = HAL_UART_STATE_READY;
    __HAL_UNLOCK(jk->huart);

    if (BMS_JK_TransmitDropEcho(jk->huart, s_jkCmds[cmdIndex], BMS_JK_CMD_LEN) != HAL_OK) {
        BMS_JK_SET_RX(jk);
        BMS_JK_SampleDirPins(jk);
        return HAL_ERROR;
    }

    /* Drop DE immediately after TC — both DIR pins low for RX. No post-TX flush. */
    BMS_JK_SET_RX(jk);
    BMS_JK_SampleDirPins(jk);

    jk->txCount++;
    return HAL_OK;
}

/**
 * Python-like collect: poll REPLY_WAIT_MS, store every RXNE/ORE byte.
 * Any residual last-byte echo lands in rxBuf; DecodeFrame finds 4E57.
 */
HAL_StatusTypeDef BMS_JK_ReceiveResponse(BMS_JK_HandleTypeDef *jk)
{
    uint16_t n = 0U;
    uint32_t t0;

    if ((jk == NULL) || (jk->huart == NULL)) {
        return HAL_ERROR;
    }

    jk->rxLen = 0U;
    jk->lastRxLen = 0U;
    memset(jk->rxHead, 0, sizeof(jk->rxHead));
    memset(jk->rxBuf, 0, sizeof(jk->rxBuf));

    BMS_JK_SET_RX(jk);
    BMS_JK_SampleDirPins(jk);
    SET_BIT(jk->huart->Instance->CR1, (USART_CR1_UE | USART_CR1_TE | USART_CR1_RE));

    t0 = HAL_GetTick();
    while ((HAL_GetTick() - t0) < BMS_JK_REPLY_WAIT_MS) {
        uint8_t b = 0U;
        if (BMS_JK_PollOneByte(jk, &b) != 0U) {
            if (n < BMS_JK_MAX_RX_BYTES) {
                jk->rxBuf[n++] = b;
            }
        }
    }

    BMS_JK_PublishRxHead(jk, n);
    BMS_JK_SampleDirPins(jk);

    if (n > 0U) {
        jk->rxCount++;
        return HAL_OK;
    }
    return HAL_BUSY;
}

/*
 * Real JK RX (jk_rx_frames.log):
 *   SOF | LEN_HI LEN_LO | 00 00 00 00 | 03 00 01 | TAG DATA... | 68 | 00 00 | CRC
 * Primary SOF = 4E57; alternate 2C54 kept as harmless fallback.
 */
static uint8_t BMS_JK_IsSof4E57(const uint8_t *buf)
{
    return ((buf[0] == 0x4EU) && (buf[1] == 0x57U)) ? 1U : 0U;
}

static uint8_t BMS_JK_IsSofAlt(const uint8_t *buf)
{
    return ((buf[0] == 0x2CU) && (buf[1] == 0x54U)) ? 1U : 0U;
}

/** Scan tags 0x85/83/84/81/87/79 in [start, end). Returns 1 if any hit. */
static uint8_t BMS_JK_ScanTags(BMS_JK_HandleTypeDef *jk, uint16_t start, uint16_t end)
{
    uint16_t pos;
    uint8_t any = 0U;

    for (pos = start; (pos + 1U) < end; pos++) {
        uint8_t tag = jk->rxBuf[pos];
        uint8_t got = 0U;

        if ((tag == 0x85U) && ((pos + 1U) < end)) {
            uint8_t val = jk->rxBuf[pos + 1U];
            if (val <= 100U) {
                jk->snapshot.soc = val;
                jk->initialized = 1U;
                got = 1U;
            }
        } else if ((tag == 0x83U) && ((pos + 2U) < end)) {
            uint16_t raw = ((uint16_t)jk->rxBuf[pos + 1U] << 8) | jk->rxBuf[pos + 2U];
            jk->snapshot.packVoltageMV = (int32_t)raw * 10;
            jk->initialized = 1U;
            got = 1U;
        } else if ((tag == 0x84U) && ((pos + 2U) < end)) {
            uint16_t raw = ((uint16_t)jk->rxBuf[pos + 1U] << 8) | jk->rxBuf[pos + 2U];
            int32_t currentMA = (int32_t)(int16_t)raw * 10;
            /* Python: if abs(A) > 500 then (10000 - unsigned) * 0.01 */
            if ((currentMA > 500000) || (currentMA < -500000)) {
                currentMA = (int32_t)(10000 - (int32_t)raw) * 10;
            }
            jk->snapshot.packCurrentMA = currentMA;
            jk->initialized = 1U;
            got = 1U;
        } else if ((tag == 0x81U) && ((pos + 2U) < end)) {
            uint16_t raw = ((uint16_t)jk->rxBuf[pos + 1U] << 8) | jk->rxBuf[pos + 2U];
            /* Python: temp = raw - 100 if raw > 100 else raw */
            jk->snapshot.mosTemperatureC =
                (raw > 100U) ? (int16_t)(raw - 100U) : (int16_t)raw;
            jk->initialized = 1U;
            got = 1U;
        } else if ((tag == 0x87U) && ((pos + 2U) < end)) {
            jk->snapshot.cycles =
                ((uint16_t)jk->rxBuf[pos + 1U] << 8) | jk->rxBuf[pos + 2U];
            jk->initialized = 1U;
            got = 1U;
        } else if ((tag == 0x79U) && ((pos + 1U) < end)) {
            uint8_t len = jk->rxBuf[pos + 1U];
            uint16_t dataStart = (uint16_t)(pos + 2U);
            uint16_t dataEnd = (uint16_t)(dataStart + len);
            uint8_t active = 0U;
            uint16_t g;

            if ((len >= 3U) && (dataEnd <= end)) {
                for (g = 0U; (g + 2U) < len; g = (uint16_t)(g + 3U)) {
                    uint8_t cellIdx = jk->rxBuf[dataStart + g];
                    uint16_t mV =
                        ((uint16_t)jk->rxBuf[dataStart + g + 1U] << 8) |
                        jk->rxBuf[dataStart + g + 2U];
                    if ((cellIdx >= 1U) && (cellIdx <= BMS_JK_MAX_CELL_COUNT) &&
                        (mV > 0U)) {
                        jk->snapshot.cellVoltageMV[cellIdx - 1U] = mV;
                        if (cellIdx > active) {
                            active = cellIdx;
                        }
                    }
                }
                if (active > 0U) {
                    jk->snapshot.cellCount = active;
        jk->initialized = 1U;
                    got = 1U;
                }
            }
        }

        if (got != 0U) {
            any = 1U;
            break; /* single-parameter replies */
        }
    }
    return any;
}

__attribute__((noinline)) void BMS_JK_DecodeFrame(BMS_JK_HandleTypeDef *jk)
{
    uint16_t i;
    uint16_t sofAt = 0xFFFFU;
    uint16_t frameLen;
    uint16_t total;

    if (jk == NULL) {
        return;
    }

    jk->decodeCount++;
    jk->sofOk = 0U;

    /* Always refresh rxHead from capture (bring-up visibility) */
    memset(jk->rxHead, 0, sizeof(jk->rxHead));
    for (i = 0U; (i < 16U) && (i < jk->rxLen); i++) {
        jk->rxHead[i] = jk->rxBuf[i];
    }

    if (jk->rxLen < 2U) {
        return; /* keep rxLen / lastRxLen */
    }

    /* Prefer classic SOF 4E57 */
    for (i = 0U; (i + 1U) < jk->rxLen; i++) {
        if (BMS_JK_IsSof4E57(&jk->rxBuf[i]) != 0U) {
            sofAt = i;
            break;
        }
    }
    /* Harmless alternate only if 4E57 absent */
    if (sofAt == 0xFFFFU) {
        for (i = 0U; (i + 1U) < jk->rxLen; i++) {
            if (BMS_JK_IsSofAlt(&jk->rxBuf[i]) != 0U) {
                sofAt = i;
                break;
            }
        }
    }

    if (sofAt == 0xFFFFU) {
        /* No SOF: do NOT wipe rxLen — raw buffer stays for Live Expressions */
        return;
    }

    if (sofAt > 0U) {
        uint16_t remain = (uint16_t)(jk->rxLen - sofAt);
        memmove(jk->rxBuf, &jk->rxBuf[sofAt], remain);
        jk->rxLen = remain;
    }

    if (jk->rxLen < 4U) {
        return;
    }

    frameLen = ((uint16_t)jk->rxBuf[2] << 8) | jk->rxBuf[3];
    total = (uint16_t)(2U + frameLen);
    if ((frameLen >= 2U) && (total <= jk->rxLen) && (total <= BMS_JK_MAX_RX_BYTES)) {
        jk->rxLen = total;
    }

    jk->sofOk = 1U;

    memset(jk->rxHead, 0, sizeof(jk->rxHead));
    for (i = 0U; (i < 16U) && (i < jk->rxLen); i++) {
        jk->rxHead[i] = jk->rxBuf[i];
    }
}

void BMS_JK_UpdateSnapshot(BMS_JK_HandleTypeDef *jk)
{
    uint16_t start;
    uint16_t end;

    if (jk == NULL) {
        return;
    }

    jk->snapshotCount++;

    if (jk->rxLen == 0U) {
        return;
    }

    end = jk->rxLen;
    if ((jk->rxLen >= 5U) && (jk->rxBuf[jk->rxLen - 5U] == 0x68U)) {
        end = (uint16_t)(jk->rxLen - 5U);
    }

    if (jk->sofOk != 0U) {
        /* Payload tags after SOF+LEN+addr+03 00 01 */
        start = 11U;
        if (start >= end) {
            start = 0U;
        }
        (void)BMS_JK_ScanTags(jk, start, end);
    } else {
        /* Loose bring-up: scan entire raw capture */
        (void)BMS_JK_ScanTags(jk, 0U, end);
    }
}

HAL_StatusTypeDef BMS_JK_Normal(BMS_JK_HandleTypeDef *jk)
{
    uint32_t now;
    uint32_t gap;
    uint8_t cmd;

    if ((jk == NULL) || (jk->huart == NULL)) {
        return HAL_ERROR;
    }

    now = HAL_GetTick();
    gap = jk->operationTick;
    if ((gap != 0U) && ((now - jk->lastPollTick) < gap)) {
        return HAL_BUSY;
    }

#if BMS_JK_BRINGUP_SOC_ONLY
    cmd = 0U; /* SOC only — default bring-up */
    jk->pollIndex = 0U;
#else
    if (jk->pollIndex >= BMS_JK_CMD_COUNT) {
        jk->pollIndex = 0U;
    }
    cmd = jk->pollIndex;
#endif

    if (BMS_JK_SendRequest(jk, cmd) != HAL_OK) {
        jk->lastPollTick = HAL_GetTick();
        jk->operationTick = BMS_JK_CMD_GAP_MS;
        return HAL_ERROR;
    }

    (void)BMS_JK_ReceiveResponse(jk);
    BMS_JK_DecodeFrame(jk);
    BMS_JK_UpdateSnapshot(jk);

#if BMS_JK_BRINGUP_SOC_ONLY
    jk->pollIndex = 0U;
    jk->operationTick = BMS_JK_CMD_GAP_MS;
#else
    jk->pollIndex++;
    if (jk->pollIndex >= BMS_JK_CMD_COUNT) {
        jk->pollIndex = 0U;
        jk->operationTick = BMS_JK_CYCLE_GAP_MS;
    } else {
        jk->operationTick = BMS_JK_CMD_GAP_MS;
    }
#endif
    jk->lastPollTick = HAL_GetTick();
    return HAL_OK;
}
