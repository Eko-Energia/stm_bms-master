#ifndef BMS_JK_H
#define BMS_JK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

#define BMS_JK_MAX_CELL_COUNT   32U
#define BMS_JK_MAX_RX_BYTES     300U
#define BMS_JK_CMD_LEN          21U

/* ---------------------------------------------------------------------------
 * RS485 Transceiver control-pin polarity.
 *
 *   Standard MAX485 / ST3485 chip pinout:
 *      DE = active HIGH (HIGH -> driver enabled)
 *      RE = active LOW  (LOW  -> receiver enabled)
 *   => For RX mode:  DE=0, RE=0
 *   => For TX mode:  DE=1, RE=1
 *
 *   Some breakout modules invert one or both of these signals on the PCB
 *   (or route DE/RE to the wrong pin). If the "textbook" polarity does not
 *   produce a valid signal on RO, flip the corresponding macro below to 1
 *   and rebuild. No rewiring required.
 * -------------------------------------------------------------------------*/
#ifndef BMS_JK_INVERT_DE
#define BMS_JK_INVERT_DE        0U   /* 0 = active HIGH (default), 1 = inverted */
#endif
#ifndef BMS_JK_INVERT_RE
#define BMS_JK_INVERT_RE        0U   /* 0 = active LOW (standard, MAX485/ST3485) */
#endif

typedef enum {
    BMS_JK_CMD_SOC = 0,
    BMS_JK_CMD_VOLTAGE,
    BMS_JK_CMD_CURRENT,
    BMS_JK_CMD_TEMPERATURE,
    BMS_JK_CMD_CELL_VOLTAGES,
    BMS_JK_CMD_CYCLES,
    BMS_JK_CMD_TOTAL_STRINGS,
    BMS_JK_CMD_WARNINGS,
    BMS_JK_CMD_STATUS,
    BMS_JK_CMD_TOTAL_OVERVOLTAGE_PROT,
    BMS_JK_CMD_TOTAL_UNDERVOLTAGE_PROT,
    BMS_JK_CMD_SINGLE_OVERVOLTAGE_PROT,
    BMS_JK_CMD_SINGLE_OVERVOLTAGE_RECOV,
    BMS_JK_CMD_DIFF_VOLTAGE_PROT,
    BMS_JK_CMD_DISCHARGE_OVERCURRENT_PROT,
    BMS_JK_CMD_CHARGE_OVERCURRENT_PROT,
    BMS_JK_CMD_BATTERY_TYPE,
    BMS_JK_CMD_ACTUAL_CAPACITY,
    BMS_JK_CMD_DEVICE_ID,
    BMS_JK_CMD_PRODUCTION_DATE,
    BMS_JK_CMD_SYSTEM_TIME,
    BMS_JK_CMD_SOFTWARE_VERSION,
    BMS_JK_CMD_COUNT
} BMS_JK_CommandID_t;

typedef enum {
    BMS_JK_TRX_MODE_RX = 0,
    BMS_JK_TRX_MODE_TX
} BMS_JK_TrxMode_t;

typedef struct {
    uint8_t cellCount;
    uint16_t cellVoltageMV[BMS_JK_MAX_CELL_COUNT];
    int32_t packVoltageMV;
    int32_t packCurrentMA;
    uint8_t soc;
    int16_t mosTemperatureC;
    int16_t balTemperatureC;
    uint32_t remainingCapacityMah;
    uint32_t fullCapacityMah;
    uint32_t cycles;
} BMS_JK_SnapshotTypeDef;

typedef enum {
    BMS_JK_STATE_IDLE = 0,
    BMS_JK_STATE_TX_WAIT,
    BMS_JK_STATE_RX_WAIT
} BMS_JK_State_t;

// Wewnątrz struktury BMS_JK_HandleTypeDef dopisz na końcu:
typedef struct {
    UART_HandleTypeDef *huart;
    GPIO_TypeDef *de_port;
    uint16_t de_pin;
    GPIO_TypeDef *re_port;
    uint16_t re_pin;
    
    BMS_JK_SnapshotTypeDef snapshot;
    uint8_t rxBuffer[BMS_JK_MAX_RX_BYTES];
    uint16_t rxLen;
    uint8_t lastReadCommand;
    uint8_t initialized;

    // Pola dla maszyny stanów non-blocking
    BMS_JK_State_t state;
    uint32_t lastPollTick;
    uint32_t operationTick;
} BMS_JK_HandleTypeDef;

// Zmieniona sygnatura (bez parametru timeout):
//HAL_StatusTypeDef BMS_JK_Normal(BMS_JK_HandleTypeDef *jk);

HAL_StatusTypeDef BMS_JK_Init(BMS_JK_HandleTypeDef *jk, UART_HandleTypeDef *huart, GPIO_TypeDef *de_port,
							  uint16_t de_pin, GPIO_TypeDef *re_port, uint16_t re_pin);

HAL_StatusTypeDef BMS_JK_DecodeFrame(BMS_JK_HandleTypeDef *jk, const uint8_t *data, uint16_t len);

void BMS_JK_ClearSnapshot(BMS_JK_HandleTypeDef *jk);

uint8_t BMS_JK_GetNextPollCommand(BMS_JK_HandleTypeDef *jk);

HAL_StatusTypeDef BMS_JK_PackAllCommandsBuffer(uint8_t *pDestBuffer, uint16_t *pTotalLen);

HAL_StatusTypeDef BMS_JK_ReceiveHandler(BMS_JK_HandleTypeDef *jk);

/**
  * @brief  Transceiver hardware self-test.
  *
  *         Sequence (blocking, ~2 seconds total):
  *           1. Toggles DE and RE pins so they can be verified on a scope
  *              directly at the transceiver IC legs.
  *           2. Enables driver AND receiver simultaneously (DE=1, RE=0) and
  *              transmits a stream of 0x55 bytes. If the transceiver is alive,
  *              its own RO output will reproduce the 0x55 pattern — visible on
  *              a scope as a clean ~baud-rate square wave.
  *           3. Restores strict RX (listening) mode.
  *
  *         Interpretation:
  *           - RO shows the 0x55 pattern  -> transceiver OK, problem is on the
  *                                            incoming A/B wiring (very likely swap).
  *           - RO stays flat at Vcc/GND   -> transceiver IC is damaged or a
  *                                            DE/RE track is broken.
  */
HAL_StatusTypeDef BMS_JK_TransceiverSelfTest(BMS_JK_HandleTypeDef *jk);

#ifdef __cplusplus
}
#endif

#endif /* BMS_JK_H */
