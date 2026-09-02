/**
  ******************************************************************************
  * @file      BMS_JK.c
  * @brief     JK-BMS Passive Receiver / Bus Sniffer with Diagnostic Counters
  ******************************************************************************
  */

#include "BMS_JK.h"
#include <string.h>
#include <stdlib.h>

// Diagnostic counters for debugging via live expressions / watch window
#define BMS_JK_DBG_LOG_SIZE   256U

typedef struct {
    uint32_t totalBytesReceived;
    uint32_t framesProcessed;
    uint32_t decodeErrors;
    uint8_t  lastRxByte;

    /* Circular log of the most recent raw bytes seen on the bus.
     * Inspect in your debugger: dbgStats.rawLog[] and dbgStats.rawLogIdx. */
    uint8_t  rawLog[BMS_JK_DBG_LOG_SIZE];
    uint16_t rawLogIdx;

    /* Snapshot of the last fully-received frame (bytes captured between
     * two 15 ms idle gaps). Inspect: dbgStats.lastFrame[], lastFrameLen. */
    uint8_t  lastFrame[BMS_JK_MAX_RX_BYTES];
    uint16_t lastFrameLen;
} BMS_JK_DebugStats_t;

static BMS_JK_DebugStats_t dbgStats = {0};

/**
  * @brief  Controls RS485 Transceiver Direction (RE/DE pins).
  *         Honors the BMS_JK_INVERT_DE / BMS_JK_INVERT_RE polarity macros
  *         so mis-wired or inverted breakout modules can be handled without
  *         changing any HW.
  */
static void BMS_JK_SetTransceiverMode(BMS_JK_HandleTypeDef *jk, BMS_JK_TrxMode_t mode)
{
    if (jk == NULL) {
        return;
    }

    /* Logical intent: driver_on = (mode == TX),  receiver_on = (mode == RX). */
    const uint8_t driver_on   = (mode == BMS_JK_TRX_MODE_TX) ? 1U : 0U;
    const uint8_t receiver_on = (mode == BMS_JK_TRX_MODE_RX) ? 1U : 0U;

    /* Standard chip: DE active-HIGH  -> pin level = driver_on
     *                RE active-LOW   -> pin level = !receiver_on
     * The macros invert either signal for non-standard breakout wiring.
     */
    GPIO_PinState de_level = driver_on   ? GPIO_PIN_SET : GPIO_PIN_RESET;
    GPIO_PinState re_level = receiver_on ? GPIO_PIN_RESET : GPIO_PIN_SET;

#if (BMS_JK_INVERT_DE != 0U)
    de_level = (de_level == GPIO_PIN_SET) ? GPIO_PIN_RESET : GPIO_PIN_SET;
#endif
#if (BMS_JK_INVERT_RE != 0U)
    re_level = (re_level == GPIO_PIN_SET) ? GPIO_PIN_RESET : GPIO_PIN_SET;
#endif

    if (jk->de_port != NULL) {
        HAL_GPIO_WritePin(jk->de_port, jk->de_pin, de_level);
    }
    if (jk->re_port != NULL) {
        HAL_GPIO_WritePin(jk->re_port, jk->re_pin, re_level);
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
    
    memset(&dbgStats, 0, sizeof(dbgStats));
    BMS_JK_ClearSnapshot(jk);

    // Explicitly lock RS485 transceiver into strict Receive (Listening) mode
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

/*
 * JK-BMS RS485 frame layout:
 *
 *   Offset  Size  Field
 *   ------  ----  ---------------------------------------------------------
 *     0     2     Start bytes         0x4E 0x57 ("NW")
 *     2     2     Length (big-endian) = total_frame_length - 2
 *     4     4     Terminal number
 *     8     1     Command word        (0x06 = read all)
 *     9     1     Frame source        (0x00=BMS, 0x01=BT, 0x02=GPS, 0x03=PC)
 *    10     1     Transport type      (0x00=request, 0x01=response, 0x02=BMS active upload)
 *    11    N-20   Frame info (TLV tags: 0x79, 0x83, 0x84, 0x85, ...)
 *   N-9     4     Record number
 *   N-5     1     End identifier      0x68
 *   N-4     2     Reserved (0x00 0x00)
 *   N-2     2     Checksum (big-endian) = sum of bytes[0..N-3] modulo 0x10000
 */

#define JK_HDR0            0x4EU
#define JK_HDR1            0x57U
#define JK_END_ID          0x68U
#define JK_MIN_FRAME_LEN   21U   /* request = 21 bytes exactly */
#define JK_TRANSPORT_OFF   10U
#define JK_INFO_START      11U
#define JK_TAIL_LEN        9U    /* record(4) + end(1) + reserved(2) + crc(2) */

static uint16_t BMS_JK_ComputeChecksum(const uint8_t *data, uint16_t len)
{
    uint32_t sum = 0U;
    for (uint16_t i = 0U; i < len; i++) {
        sum += data[i];
    }
    return (uint16_t)(sum & 0xFFFFU);
}

HAL_StatusTypeDef BMS_JK_DecodeFrame(BMS_JK_HandleTypeDef *jk, const uint8_t *data, uint16_t len)
{
    int data_parsed = 0;

    if ((jk == NULL) || (data == NULL) || (len < JK_MIN_FRAME_LEN)) {
        dbgStats.decodeErrors++;
        return HAL_ERROR;
    }

    /* --- 1. Validate the JK magic header --- */
    if ((data[0] != JK_HDR0) || (data[1] != JK_HDR1)) {
        dbgStats.decodeErrors++;
        return HAL_ERROR;
    }

    /* --- 2. Validate the length field --- */
    uint16_t lenField = ((uint16_t)data[2] << 8U) | data[3];
    if ((uint32_t)lenField + 2U != (uint32_t)len) {
        /* Length mismatch: either partial capture or garbled frame */
        dbgStats.decodeErrors++;
        return HAL_ERROR;
    }

    /* --- 3. Validate the checksum --- */
    uint16_t calcCrc = BMS_JK_ComputeChecksum(data, (uint16_t)(len - 2U));
    uint16_t frameCrc = ((uint16_t)data[len - 2] << 8U) | data[len - 1];
    if (calcCrc != frameCrc) {
        dbgStats.decodeErrors++;
        return HAL_ERROR;
    }

    /* --- 4. Discriminate request vs response ---
     * Only responses (transport type 0x01) and BMS active uploads (0x02)
     * contain telemetry TLVs worth decoding.
     */
    uint8_t transport = data[JK_TRANSPORT_OFF];
    if (transport != 0x01U && transport != 0x02U) {
        /* Request frame (0x00) – nothing to decode, but it's a valid frame */
        dbgStats.framesProcessed++;
        return HAL_OK;
    }

    /* --- 5. Parse tags, restricted to the frame-info region --- */
    const uint16_t infoStart = JK_INFO_START;
    const uint16_t infoEnd   = (uint16_t)(len - JK_TAIL_LEN);   /* exclusive */

    if (infoEnd <= infoStart) {
        dbgStats.decodeErrors++;
        return HAL_ERROR;
    }

    uint16_t i;
    for (i = infoStart; i < infoEnd; i++) {
        uint8_t tag = data[i];

        if (tag == 0x79U) {
            /* Cell voltages: [0x79][N][cell#][mvH][mvL] * (N/3) */
            if (i + 1U >= infoEnd) break;
            uint8_t cell_data_len = data[i + 1];
            uint8_t num_cells = cell_data_len / 3U;
            if ((num_cells > 0U) && (num_cells <= BMS_JK_MAX_CELL_COUNT) &&
                ((uint32_t)i + 2U + cell_data_len <= infoEnd)) {
                jk->snapshot.cellCount = num_cells;
                int32_t total_mv = 0;
                for (uint8_t c = 0U; c < num_cells; c++) {
                    uint16_t mv = ((uint16_t)data[i + 2U + (c * 3U) + 1U] << 8U)
                                |            data[i + 2U + (c * 3U) + 2U];
                    jk->snapshot.cellVoltageMV[c] = mv;
                    total_mv += (int32_t)mv;
                }
                if (jk->snapshot.packVoltageMV == 0) {
                    jk->snapshot.packVoltageMV = total_mv;
                }
                data_parsed = 1;
            }
            /* Advance past tag(1) + length(1) + payload; outer i++ does the rest */
            i += (uint16_t)(1U + cell_data_len);
        }
        else if (tag == 0x80U || tag == 0x81U || tag == 0x82U) {
            /* Temperatures: [tag][hi][lo], value in 0.1 °C, offset 100 */
            if (i + 2U >= infoEnd) break;
            uint16_t raw = ((uint16_t)data[i + 1] << 8U) | data[i + 2];
            int16_t temp = (raw > 100U) ? (int16_t)(raw - 100U) : (int16_t)(-(int32_t)raw);
            if (tag == 0x81U) {
                jk->snapshot.mosTemperatureC = temp;
            } else if (tag == 0x82U) {
                jk->snapshot.balTemperatureC = temp;
            }
            data_parsed = 1;
            i += 2U;
        }
        else if (tag == 0x83U) {
            /* Pack voltage: [0x83][hi][lo], unit 10 mV -> mV */
            if (i + 2U >= infoEnd) break;
            uint16_t raw = ((uint16_t)data[i + 1] << 8U) | data[i + 2];
            jk->snapshot.packVoltageMV = (int32_t)raw * 10;
            data_parsed = 1;
            i += 2U;
        }
        else if (tag == 0x84U) {
            /* Pack current: [0x84][hi][lo], signed 10 mA -> mA
             * Some firmwares encode as unsigned offset (10000 - x). */
            if (i + 2U >= infoEnd) break;
            uint16_t raw_u = ((uint16_t)data[i + 1] << 8U) | data[i + 2];
            int16_t  raw_s = (int16_t)raw_u;
            int32_t  current_mA = (int32_t)raw_s * 10;
            if (abs((int)current_mA) > 500000) {
                current_mA = (int32_t)(10000 - (int32_t)raw_u) * 10;
            }
            jk->snapshot.packCurrentMA = current_mA;
            data_parsed = 1;
            i += 2U;
        }
        else if (tag == 0x85U) {
            /* SoC %: [0x85][value] */
            if (i + 1U >= infoEnd) break;
            uint8_t soc = data[i + 1];
            if (soc <= 100U) {
                jk->snapshot.soc = soc;
                data_parsed = 1;
            }
            i += 1U;
        }
        else if (tag == 0x86U) {
            /* Number of temperature sensors: [0x86][count] */
            if (i + 1U >= infoEnd) break;
            i += 1U;
        }
        else if (tag == 0x87U) {
            /* Cycle count: [0x87][hi][lo] */
            if (i + 2U >= infoEnd) break;
            uint16_t cycles = ((uint16_t)data[i + 1] << 8U) | data[i + 2];
            jk->snapshot.cycles = cycles;
            data_parsed = 1;
            i += 2U;
        }
        else if (tag == 0x89U) {
            /* Total cycle capacity: [0x89][b3][b2][b1][b0] */
            if (i + 4U >= infoEnd) break;
            i += 4U;
        }
        /* Unknown tag: skip a single byte via the outer for-loop increment. */
    }

    if (data_parsed != 0) {
        jk->initialized = 1U;
        dbgStats.framesProcessed++;
        return HAL_OK;
    }

    dbgStats.decodeErrors++;
    return HAL_ERROR;
}

/**
  * @brief  Non-blocking background receiver to be called continuously in main loop.
  *         Catches incoming traffic on the RS485 bus, buffers bytes, and decodes
  *         responses when a packet gap or buffer limit is reached.
  */
HAL_StatusTypeDef BMS_JK_ReceiveHandler(BMS_JK_HandleTypeDef *jk)
{
    if (jk == NULL || jk->huart == NULL) {
        return HAL_ERROR;
    }

    // Ensure we stay in RX (listening) mode every iteration
    BMS_JK_SetTransceiverMode(jk, BMS_JK_TRX_MODE_RX);

    // Clear any pending overrun / framing / noise errors that would otherwise
    // freeze RXNE on STM32F1 (ORE requires SR read then DR read to clear).
    if (__HAL_UART_GET_FLAG(jk->huart, UART_FLAG_ORE) != RESET ||
        __HAL_UART_GET_FLAG(jk->huart, UART_FLAG_FE)  != RESET ||
        __HAL_UART_GET_FLAG(jk->huart, UART_FLAG_NE)  != RESET) {
        volatile uint32_t tmp;
        tmp = jk->huart->Instance->SR;
        tmp = jk->huart->Instance->DR;
        (void)tmp;
    }

    // Make sure UART receiver is actually enabled (RE bit in CR1)
    if ((jk->huart->Instance->CR1 & USART_CR1_RE) == 0U) {
        jk->huart->Instance->CR1 |= USART_CR1_RE;
    }

    /* ------------------------------------------------------------------
     * Byte-level RX state machine (header sync + length-based framing):
     *
     *   State 0: waiting for first header byte 0x4E
     *   State 1: waiting for second header byte 0x57
     *   State 2: collecting the two length bytes
     *   State 3: collecting the remaining (len_field) bytes of the frame
     *
     * This is far more robust than the previous "wait for 15 ms idle then
     * try to decode whatever is in the buffer" approach. It:
     *   - Recovers instantly from garbage / partial frames.
     *   - Correctly separates back-to-back frames.
     *   - Cannot overflow the buffer (length is validated).
     * ------------------------------------------------------------------ */
    static uint8_t  rxState        = 0U;
    static uint16_t rxFrameLen     = 0U;  /* total expected frame length (bytes) */
    static uint16_t rxCollected    = 0U;
    HAL_StatusTypeDef result       = HAL_BUSY;

    while (__HAL_UART_GET_FLAG(jk->huart, UART_FLAG_RXNE) != RESET) {
        uint8_t rx_byte = (uint8_t)(jk->huart->Instance->DR & 0xFFU);

        dbgStats.totalBytesReceived++;
        dbgStats.lastRxByte = rx_byte;
        dbgStats.rawLog[dbgStats.rawLogIdx] = rx_byte;
        dbgStats.rawLogIdx = (uint16_t)((dbgStats.rawLogIdx + 1U) % BMS_JK_DBG_LOG_SIZE);

        switch (rxState) {
            case 0U:  /* hunting for 0x4E */
                if (rx_byte == JK_HDR0) {
                    jk->rxBuffer[0] = rx_byte;
                    rxCollected = 1U;
                    rxState = 1U;
                }
                break;

            case 1U:  /* expecting 0x57 immediately after 0x4E */
                if (rx_byte == JK_HDR1) {
                    jk->rxBuffer[1] = rx_byte;
                    rxCollected = 2U;
                    rxState = 2U;
                } else if (rx_byte == JK_HDR0) {
                    /* stay in state 1: latest 0x4E is the new potential header */
                    jk->rxBuffer[0] = JK_HDR0;
                    rxCollected = 1U;
                } else {
                    /* not a header, resync */
                    rxState = 0U;
                    rxCollected = 0U;
                }
                break;

            case 2U:  /* collecting the 2-byte length field */
                jk->rxBuffer[rxCollected++] = rx_byte;
                if (rxCollected == 4U) {
                    uint16_t lenField = ((uint16_t)jk->rxBuffer[2] << 8U) | jk->rxBuffer[3];
                    rxFrameLen = (uint16_t)(lenField + 2U);  /* total incl. header */
                    if ((rxFrameLen < JK_MIN_FRAME_LEN) || (rxFrameLen > BMS_JK_MAX_RX_BYTES)) {
                        /* Absurd length -> resync */
                        dbgStats.decodeErrors++;
                        rxState = 0U;
                        rxCollected = 0U;
                    } else {
                        rxState = 3U;
                    }
                }
                break;

            case 3U:  /* collecting frame body up to rxFrameLen */
                jk->rxBuffer[rxCollected++] = rx_byte;
                if (rxCollected >= rxFrameLen) {
                    /* Frame complete: snapshot for debugger, then decode */
                    memcpy(dbgStats.lastFrame, jk->rxBuffer, rxFrameLen);
                    dbgStats.lastFrameLen = rxFrameLen;

                    result = BMS_JK_DecodeFrame(jk, jk->rxBuffer, rxFrameLen);

                    /* Reset for next frame */
                    rxState = 0U;
                    rxCollected = 0U;
                    rxFrameLen = 0U;
                    jk->rxLen = 0U;
                }
                break;

            default:
                rxState = 0U;
                rxCollected = 0U;
                break;
        }
    }

    return result;
}

HAL_StatusTypeDef BMS_JK_TransceiverSelfTest(BMS_JK_HandleTypeDef *jk)
{
    if (jk == NULL || jk->huart == NULL) {
        return HAL_ERROR;
    }

    /* --- Phase 1: slow DE/RE toggle so wiring can be verified on a scope --- */
    for (uint8_t i = 0; i < 5U; i++) {
        BMS_JK_SetTransceiverMode(jk, BMS_JK_TRX_MODE_TX);   /* DE=1, RE=1 */
        HAL_Delay(100);
        BMS_JK_SetTransceiverMode(jk, BMS_JK_TRX_MODE_RX);   /* DE=0, RE=0 */
        HAL_Delay(100);
    }

    /* --- Phase 2: local loopback through the transceiver ---
     * Enable the driver AND keep the receiver enabled so RO reflects what the
     * driver is putting on A/B. 0x55 is chosen because it produces a maximum
     * transition-density square wave (10101010b + start/stop bits).
     */
    if (jk->de_port != NULL) {
        HAL_GPIO_WritePin(jk->de_port, jk->de_pin, GPIO_PIN_SET);   /* driver ON  */
    }
    if (jk->re_port != NULL) {
        HAL_GPIO_WritePin(jk->re_port, jk->re_pin, GPIO_PIN_RESET); /* receiver ON */
    }

    uint8_t pattern[16];
    memset(pattern, 0x55, sizeof(pattern));

    uint32_t start = HAL_GetTick();
    while ((HAL_GetTick() - start) < 1000U) {
        (void)HAL_UART_Transmit(jk->huart, pattern, sizeof(pattern), 100U);
    }

    /* --- Phase 3: restore strict RX (listening) mode --- */
    BMS_JK_SetTransceiverMode(jk, BMS_JK_TRX_MODE_RX);

    /* Flush anything the loopback pushed into the RX register */
    if (__HAL_UART_GET_FLAG(jk->huart, UART_FLAG_ORE) != RESET) {
        volatile uint32_t tmp;
        tmp = jk->huart->Instance->SR;
        tmp = jk->huart->Instance->DR;
        (void)tmp;
    }
    while (__HAL_UART_GET_FLAG(jk->huart, UART_FLAG_RXNE) != RESET) {
        (void)jk->huart->Instance->DR;
    }
    jk->rxLen = 0;

    return HAL_OK;
}
