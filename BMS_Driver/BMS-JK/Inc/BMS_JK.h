#ifndef BMS_JK_H
#define BMS_JK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

#define BMS_JK_MAX_CELL_COUNT   32U
#define BMS_JK_MAX_RX_BYTES     300U
#define BMS_JK_CMD_LEN          21U
#define BMS_JK_CMD_COUNT        22U

/* Same timings as bms_jk_sender_receiver.py */
#define BMS_JK_REPLY_WAIT_MS    250U
#define BMS_JK_CMD_GAP_MS        50U
#define BMS_JK_CYCLE_GAP_MS    5000U

typedef struct {
    uint8_t  cellCount;
    uint16_t cellVoltageMV[BMS_JK_MAX_CELL_COUNT];
    int32_t  packVoltageMV;
    int32_t  packCurrentMA;
    uint8_t  soc;
    int16_t  mosTemperatureC;
    int16_t  balTemperatureC;
    uint32_t cycles;
} BMS_JK_SnapshotTypeDef;

typedef struct {
    UART_HandleTypeDef *huart;
    BMS_JK_SnapshotTypeDef snapshot;

    uint8_t  rxBuf[BMS_JK_MAX_RX_BYTES];
    uint16_t rxLen;
    uint8_t  pollIndex;
    uint8_t  initialized;
    uint8_t  sofOk;        /* 1 = rxBuf aligned on valid 4E57 frame */
    uint32_t lastPollTick;
    uint32_t operationTick;

    /* Live Expressions */
    uint32_t txCount;
    uint32_t rxCount;
    uint16_t lastRxLen;
    uint32_t feCount;      /* framing/noise clears during RX poll */
    uint32_t oreCount;     /* overrun seen during RX poll */
    uint32_t decodeCount;  /* BMS_JK_DecodeFrame invocations */
    uint32_t snapshotCount;/* BMS_JK_UpdateSnapshot invocations */
} BMS_JK_HandleTypeDef;

HAL_StatusTypeDef BMS_JK_Init(BMS_JK_HandleTypeDef *jk, UART_HandleTypeDef *huart,
                              GPIO_TypeDef *de_port, uint16_t de_pin,
                              GPIO_TypeDef *re_port, uint16_t re_pin);

void BMS_JK_SET_TX(BMS_JK_HandleTypeDef *jk);
void BMS_JK_SET_RX(BMS_JK_HandleTypeDef *jk);

HAL_StatusTypeDef BMS_JK_SendRequest(BMS_JK_HandleTypeDef *jk, uint8_t cmdIndex);
HAL_StatusTypeDef BMS_JK_ReceiveResponse(BMS_JK_HandleTypeDef *jk);

/**
 * Align rxBuf on SOF 4E57 (observed on every real JK RX), validate length,
 * set sofOk. Keeps lastRxLen even when SOF is missing (noinline for breakpoints).
 */
void BMS_JK_DecodeFrame(BMS_JK_HandleTypeDef *jk);
/** Extract SOC/V/I/T/cells/cycles from real single-param JK replies (Python layout). */
void BMS_JK_UpdateSnapshot(BMS_JK_HandleTypeDef *jk);

/* Name aliases used earlier in bring-up */
#define BMS_JK_DecodeResponse BMS_JK_DecodeFrame
#define BMS_JK_DecodeReponse  BMS_JK_DecodeFrame

/**
  * One Python-paced cmd: send → poll-RX 250 ms → decode → update snapshot.
  */
HAL_StatusTypeDef BMS_JK_Normal(BMS_JK_HandleTypeDef *jk);

#ifdef __cplusplus
}
#endif

#endif /* BMS_JK_H */
