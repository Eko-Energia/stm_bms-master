#ifndef BMS_JK_H
#define BMS_JK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

#define BMS_JK_MAX_CELL_COUNT   32U
#define BMS_JK_MAX_RX_BYTES     300U
#define BMS_JK_CMD_LEN          21U

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
} BMS_JK_HandleTypeDef;

HAL_StatusTypeDef BMS_JK_Init(BMS_JK_HandleTypeDef *jk, UART_HandleTypeDef *huart, 
                              GPIO_TypeDef *de_port, uint16_t de_pin, 
                              GPIO_TypeDef *re_port, uint16_t re_pin);
HAL_StatusTypeDef BMS_JK_Normal(BMS_JK_HandleTypeDef *jk, uint32_t timeout);
HAL_StatusTypeDef BMS_JK_DecodeFrame(BMS_JK_HandleTypeDef *jk, const uint8_t *data, uint16_t len);
void BMS_JK_ClearSnapshot(BMS_JK_HandleTypeDef *jk);

uint8_t BMS_JK_GetNextPollCommand(BMS_JK_HandleTypeDef *jk);
HAL_StatusTypeDef BMS_JK_PackAllCommandsBuffer(uint8_t *pDestBuffer, uint16_t *pTotalLen);

#ifdef __cplusplus
}
#endif

#endif /* BMS_JK_H */