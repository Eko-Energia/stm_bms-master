/**
  ******************************************************************************
  * @file    BMS_JK.c
  * @brief   Direct C port of bms_jk_sender_receiver.py
  *
  * Python:
  *   for name, cmd_hex in COMMANDS.items():
  *       ser.reset_input_buffer()
  *       ser.write(bytes.fromhex(cmd_hex))   # 21 bytes
  *       time.sleep(0.25)
  *       response = ser.read(ser.in_waiting)
  *       parse...
  *       time.sleep(0.05)
  *   time.sleep(5)
  *
 * RS485: DE (RS_DIR) high only while shifting TX; drop DE immediately after TC so the
 * bus can return idle-high. /RE stays enabled (RO driven). Local TX echo is drained
 * during TX. After turnaround, BMS_JK_POST_TX_DELAY_MS settle then 250 ms collect
 * (F1 RX FIFO is 1 deep — settle still reads so the JK reply is not overrun).
 ******************************************************************************
 */

#include "BMS_JK.h"
#include "main.h"
#include <string.h>

/*
 * COMMANDS = {
 *   "SOC":            "4E57001300000000030300850000000068000001AB",
 *   "Voltage":        "4E57001300000000030300830000000068000001A9",
 *   ...
 * }
 */
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
    /* /RE stays LOW — RO remains driven (no floating / "high-pass" look) */
    HAL_GPIO_WritePin(RE_DIR_GPIO_Port, RE_DIR_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(RS_DIR_GPIO_Port, RS_DIR_Pin, GPIO_PIN_SET);
}

void BMS_JK_SET_RX(BMS_JK_HandleTypeDef *jk)
{
    (void)jk;
    /* DE low first — release bus; idle must return HIGH (not stuck 0) */
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

/** TX one frame while discarding local RS485 echo on RO (keeps DR free). */
static HAL_StatusTypeDef BMS_JK_TransmitDropEcho(UART_HandleTypeDef *huart,
                                                 const uint8_t *data, uint16_t len)
{
    uint16_t i;
    uint32_t guard;

    for (i = 0U; i < len; i++) {
        guard = 0U;
        while (__HAL_UART_GET_FLAG(huart, UART_FLAG_TXE) == RESET) {
            if (__HAL_UART_GET_FLAG(huart, UART_FLAG_RXNE) != RESET) {
                (void)huart->Instance->DR; /* drop echo */
            }
            if (++guard > 500000U) {
                return HAL_TIMEOUT;
            }
        }
        huart->Instance->DR = data[i];

        /* Echo of this byte arrives shortly — drain it */
        guard = 0U;
        while (guard++ < 50000U) {
            if (__HAL_UART_GET_FLAG(huart, UART_FLAG_RXNE) != RESET) {
                (void)huart->Instance->DR;
                break;
            }
        }
    }

    guard = 0U;
    while (__HAL_UART_GET_FLAG(huart, UART_FLAG_TC) == RESET) {
        if (__HAL_UART_GET_FLAG(huart, UART_FLAG_RXNE) != RESET) {
            (void)huart->Instance->DR;
        }
        if (++guard > 500000U) {
            return HAL_TIMEOUT;
        }
    }

    /* Do not flush here — caller drops DE first, then clears residual echo */
    return HAL_OK;
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

    if (BMS_JK_TransmitDropEcho(jk->huart, s_jkCmds[cmdIndex], BMS_JK_CMD_LEN) != HAL_OK) {
        BMS_JK_SET_RX(jk);
        BMS_JK_SampleDirPins(jk);
        return HAL_ERROR;
    }

    /* Drop DE immediately after TC so A/B / RO can return idle-high (not stuck 0). */
    BMS_JK_SET_RX(jk);
    BMS_JK_SampleDirPins(jk);

    /* Clear residual TX echo / DE-edge glitch only — do not eat the JK reply. */
    BMS_JK_FlushRx(jk->huart);

    jk->txCount++;
    return HAL_OK;
}

/*
 * Post-TX settle (BMS_JK_POST_TX_DELAY_MS) then reply collect (BMS_JK_REPLY_WAIT_MS).
 * Both phases read continuously — F1 RX FIFO depth is 1; blind delay after TX
 * would overrun and lose SOF. FE/NE counted and discarded (often 0x00) so they
 * cannot poison the leading edge of rxBuf / sofOk; clean bytes are stored.
 */
HAL_StatusTypeDef BMS_JK_ReceiveResponse(BMS_JK_HandleTypeDef *jk)
{
    uint16_t n = 0U;
    uint16_t i;
    uint32_t t0;
    uint32_t phase;

    if ((jk == NULL) || (jk->huart == NULL)) {
        return HAL_ERROR;
    }

    jk->rxLen = 0U;
    memset(jk->rxHead, 0, sizeof(jk->rxHead));
    BMS_JK_SET_RX(jk);
    BMS_JK_SampleDirPins(jk);
    SET_BIT(jk->huart->Instance->CR1, USART_CR1_RE);

    for (phase = 0U; phase < 2U; phase++) {
        uint32_t phaseMs = (phase == 0U) ? BMS_JK_POST_TX_DELAY_MS : BMS_JK_REPLY_WAIT_MS;
        t0 = HAL_GetTick();
        while ((HAL_GetTick() - t0) < phaseMs) {
            uint32_t sr = jk->huart->Instance->SR;

            if ((sr & (USART_SR_RXNE | USART_SR_ORE)) != 0U) {
                uint8_t b = (uint8_t)(jk->huart->Instance->DR & 0xFFU);
                if ((sr & USART_SR_ORE) != 0U) {
                    jk->oreCount++;
                }
                if ((sr & (USART_SR_FE | USART_SR_NE)) != 0U) {
                    /* Clear error via DR read above; do not store garbage */
                    jk->feCount++;
                    continue;
                }
                if (n < BMS_JK_MAX_RX_BYTES) {
                    jk->rxBuf[n++] = b;
                }
            } else if ((sr & (USART_SR_FE | USART_SR_NE)) != 0U) {
                (void)jk->huart->Instance->DR;
                jk->feCount++;
            }
        }
    }

    jk->rxLen = n;
    jk->lastRxLen = n;
    for (i = 0U; (i < 16U) && (i < n); i++) {
        jk->rxHead[i] = jk->rxBuf[i];
    }

    if (n > 0U) {
        jk->rxCount++;
        return HAL_OK;
    }
    return HAL_BUSY;
}

/*
 * Real JK RX (from jk_rx_frames.log):
 *   SOF | LEN_HI LEN_LO | 00 00 00 00 | 03 00 01 | TAG [DATA...] | 68 | 00 00 | CRC
 * SOF = 4E57 (classic/Python) or 2C54 (alternate).
 * LEN = byte count from LEN field through CRC (total frame = 2 + LEN).
 * Short replies are single-parameter; CellVoltages is a longer 0x79 dump.
 */
/** True if buf[0..1] is a recognized JK SOF (4E57 or 2C54). */
static uint8_t BMS_JK_IsSof(const uint8_t *buf)
{
    if (buf == NULL) {
        return 0U;
    }
    if ((buf[0] == 0x4EU) && (buf[1] == 0x57U)) {
        return 1U; /* classic Python / jk_rx_frames.log */
    }
    if ((buf[0] == 0x2CU) && (buf[1] == 0x54U)) {
        return 1U; /* alternate SOF */
    }
    return 0U;
}

__attribute__((noinline)) void BMS_JK_DecodeFrame(BMS_JK_HandleTypeDef *jk)
{
    uint16_t i;
    uint16_t frameLen;
    uint16_t total;

    if (jk == NULL) {
        return;
    }

    jk->decodeCount++;
    jk->sofOk = 0U;

    if (jk->rxLen < 4U) {
        /* Keep lastRxLen from ReceiveResponse; nothing usable */
        return;
    }

    /* Find SOF 4E57 or 2C54 — skip leading garbage */
    for (i = 0U; (i + 1U) < jk->rxLen; i++) {
        if (BMS_JK_IsSof(&jk->rxBuf[i]) != 0U) {
            if (i > 0U) {
                uint16_t remain = (uint16_t)(jk->rxLen - i);
                memmove(jk->rxBuf, &jk->rxBuf[i], remain);
                jk->rxLen = remain;
            }
            break;
        }
    }

    if ((jk->rxLen < 4U) || (BMS_JK_IsSof(jk->rxBuf) == 0U)) {
        /* No SOF: do NOT wipe lastRxLen; clear rxLen so UpdateSnapshot skips */
        jk->rxLen = 0U;
        return;
    }

    /* Trim to declared length when the length field is consistent */
    frameLen = ((uint16_t)jk->rxBuf[2] << 8) | jk->rxBuf[3];
    total = (uint16_t)(2U + frameLen);
    if ((frameLen >= 2U) && (total <= jk->rxLen) && (total <= BMS_JK_MAX_RX_BYTES)) {
        jk->rxLen = total;
    }

    jk->sofOk = 1U;

    /* Live Expressions: show aligned SOF, not pre-SOF noise */
    memset(jk->rxHead, 0, sizeof(jk->rxHead));
    for (i = 0U; (i < 16U) && (i < jk->rxLen); i++) {
        jk->rxHead[i] = jk->rxBuf[i];
    }
}

/* Match parse_parameter() / parse_cell_voltages() — scan payload after 03 00 01 header */
void BMS_JK_UpdateSnapshot(BMS_JK_HandleTypeDef *jk)
{
    uint16_t pos;
    uint16_t start;
    uint16_t end;

    if (jk == NULL) {
        return;
    }

    jk->snapshotCount++;

    if ((jk->sofOk == 0U) || (jk->rxLen < 12U)) {
        return;
    }
    /* Accept either SOF; still try Python-style tag scan (same layout assumed) */
    if (BMS_JK_IsSof(jk->rxBuf) == 0U) {
        return;
    }

    /* Payload tags begin at offset 11 (after SOF+LEN+addr+03 00 01) */
    start = 11U;
    if (start >= jk->rxLen) {
        start = 0U;
    }

    /* Trailer is 68 00 00 CRC CRC — locate 68 from the end (avoid mid-payload hits) */
    end = jk->rxLen;
    if ((jk->rxLen >= 5U) && (jk->rxBuf[jk->rxLen - 5U] == 0x68U)) {
        end = (uint16_t)(jk->rxLen - 5U);
    }

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
            jk->snapshot.packVoltageMV = (int32_t)raw * 10; /* 0.01 V → mV */
            jk->initialized = 1U;
            got = 1U;
        } else if ((tag == 0x84U) && ((pos + 2U) < end)) {
            uint16_t raw = ((uint16_t)jk->rxBuf[pos + 1U] << 8) | jk->rxBuf[pos + 2U];
            int32_t currentMA = (int32_t)(int16_t)raw * 10; /* signed 0.01 A → mA */
            /* Python: if abs(A) > 500 then (10000 - unsigned) * 0.01 */
            if ((currentMA > 500000) || (currentMA < -500000)) {
                currentMA = (int32_t)(10000 - (int32_t)raw) * 10;
            }
            jk->snapshot.packCurrentMA = currentMA;
            jk->initialized = 1U;
            got = 1U;
        } else if ((tag == 0x81U) && ((pos + 2U) < end)) {
            uint16_t raw = ((uint16_t)jk->rxBuf[pos + 1U] << 8) | jk->rxBuf[pos + 2U];
            /* Python: temp = raw - 100 if raw > 100 else raw  (NOT negate) */
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

        /* Real replies are single-parameter — stop after first hit */
        if (got != 0U) {
            break;
        }
    }
}

/*
 * One COMMANDS entry per call (same pacing as Python for-loop body).
 * Full table cycle then 5 s gap.
 */
HAL_StatusTypeDef BMS_JK_Normal(BMS_JK_HandleTypeDef *jk)
{
    uint32_t now;
    uint32_t gap;

    if ((jk == NULL) || (jk->huart == NULL)) {
        return HAL_ERROR;
    }

    now = HAL_GetTick();
    gap = jk->operationTick;
    if ((gap != 0U) && ((now - jk->lastPollTick) < gap)) {
        return HAL_BUSY;
    }

    if (jk->pollIndex >= BMS_JK_CMD_COUNT) {
        jk->pollIndex = 0U;
    }

    /* write → post-TX settle (50 ms) → collect 250 ms → parse; gap 50 ms / 5 s */
    if (BMS_JK_SendRequest(jk, jk->pollIndex) != HAL_OK) {
        jk->lastPollTick = HAL_GetTick();
        jk->operationTick = BMS_JK_CMD_GAP_MS;
        return HAL_ERROR;
    }

    /* ReceiveResponse: BMS_JK_POST_TX_DELAY_MS then BMS_JK_REPLY_WAIT_MS */
    (void)BMS_JK_ReceiveResponse(jk);
    BMS_JK_DecodeFrame(jk);
    BMS_JK_UpdateSnapshot(jk);

    jk->pollIndex++;
    if (jk->pollIndex >= BMS_JK_CMD_COUNT) {
        jk->pollIndex = 0U;
        jk->operationTick = BMS_JK_CYCLE_GAP_MS; /* sleep(5) */
    } else {
        jk->operationTick = BMS_JK_CMD_GAP_MS;   /* sleep(0.05) */
    }
    jk->lastPollTick = HAL_GetTick();
    return HAL_OK;
}
