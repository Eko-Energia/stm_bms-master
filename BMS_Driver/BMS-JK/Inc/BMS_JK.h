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

/*
 * Bring-up: receive ANYTHING — store every USART byte, keep raw buffer even
 * without SOF, best-effort tag parse. HSI-only — do not enable HSE.
 */
#ifndef BMS_JK_BRINGUP_SOC_ONLY
#define BMS_JK_BRINGUP_SOC_ONLY     1   /* poll SOC (cmd 0) only — easier scope sync */
#endif
#ifndef BMS_JK_BRINGUP_USE_HAL_RX
#define BMS_JK_BRINGUP_USE_HAL_RX   0   /* 0 = polled store-all (FE counted); 1 = HAL */
#endif

/* Same pacing as Python; bring-up uses a longer reply window */
#define BMS_JK_POST_TX_DELAY_MS    5U   /* short DE/bus settle; collect during this too */
#if BMS_JK_BRINGUP_SOC_ONLY
#define BMS_JK_REPLY_WAIT_MS     500U   /* long window for scope + slow JK */
#else
#define BMS_JK_REPLY_WAIT_MS     250U   /* Python time.sleep(0.25) */
#endif
#define BMS_JK_CMD_GAP_MS         50U
#define BMS_JK_CYCLE_GAP_MS     5000U
#define BMS_JK_ECHO_DRAIN_MS       2U   /* drain residual TX echo only; do not eat JK SOF */

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
    uint8_t  sofOk;        /* 1 = rxBuf aligned on SOF 4E57 or 2C54 */
    uint32_t lastPollTick;
    uint32_t operationTick;

    /* Live Expressions */
    uint32_t txCount;
    uint32_t rxCount;
    uint16_t lastRxLen;
    uint8_t  rxHead[16];   /* first bytes of last RX (no SOF breakpoint needed) */
    uint32_t feCount;      /* framing/noise seen during RX (bytes may still be stored) */
    uint32_t oreCount;     /* overrun seen during RX poll */
    uint32_t decodeCount;  /* BMS_JK_DecodeFrame invocations */
    uint32_t snapshotCount;/* BMS_JK_UpdateSnapshot invocations */
    uint8_t  dePin;        /* RS_DIR level after turnaround (0 = RX) */
    uint8_t  rePin;        /* RE_DIR level (0 = /RE enabled) */
    uint16_t cleanRxLen;   /* bytes stored without FE/NE on that read */
} BMS_JK_HandleTypeDef;

HAL_StatusTypeDef BMS_JK_Init(BMS_JK_HandleTypeDef *jk, UART_HandleTypeDef *huart,
                              GPIO_TypeDef *de_port, uint16_t de_pin,
                              GPIO_TypeDef *re_port, uint16_t re_pin);

void BMS_JK_SET_TX(BMS_JK_HandleTypeDef *jk);
void BMS_JK_SET_RX(BMS_JK_HandleTypeDef *jk);

HAL_StatusTypeDef BMS_JK_SendRequest(BMS_JK_HandleTypeDef *jk, uint8_t cmdIndex);
HAL_StatusTypeDef BMS_JK_ReceiveResponse(BMS_JK_HandleTypeDef *jk);

/**
 * Try SOF 4E57/2C54 align → sofOk=1. If no SOF: keep rxLen/rxBuf/rxHead intact
 * (sofOk=0) so raw USART data stays visible for bring-up.
 */
void BMS_JK_DecodeFrame(BMS_JK_HandleTypeDef *jk);
/**
 * sofOk: structured tag scan. Else if rxLen>0: loose tag scan over raw buffer.
 * Never clears rxBuf for bring-up visibility.
 */
void BMS_JK_UpdateSnapshot(BMS_JK_HandleTypeDef *jk);

/* Name aliases used earlier in bring-up */
#define BMS_JK_DecodeResponse BMS_JK_DecodeFrame
#define BMS_JK_DecodeReponse  BMS_JK_DecodeFrame

/**
  * One Python-paced cmd: send → post-TX settle → RX → decode → snapshot.
  */
HAL_StatusTypeDef BMS_JK_Normal(BMS_JK_HandleTypeDef *jk);

#ifdef __cplusplus
}
#endif

#endif /* BMS_JK_H */
