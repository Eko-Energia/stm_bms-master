/**
  ******************************************************************************
  * @file    BMS_JK.h
  * @author  AGH Eko-Energy
  * @brief   JK BMS UART/CAN bridge interface.
  ******************************************************************************
  */

#ifndef BMS_JK_H_
#define BMS_JK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "stm32f1xx_hal.h"
#include "main.h"

/* ---------------------------------------------------------------------------
   JK request/register IDs from the verified BMS_JK_ReadingAlgo protocol
   --------------------------------------------------------------------------- */
#define BMS_JK_CMD_CELL_VOLTAGES   (0x79U)
#define BMS_JK_CMD_TEMPERATURES    (0x81U)
#define BMS_JK_CMD_VOLTAGE         (0x83U)
#define BMS_JK_CMD_CURRENT         (0x84U)
#define BMS_JK_CMD_SOC             (0x85U)
#define BMS_JK_CMD_CAPACITY        (0xAAU)
#define BMS_JK_CMD_ALL             (0x79U)
#define BMS_JK_POLL_CMD_COUNT      (6U)

#define BMS_JK_FLAG_VALID_RESPONSE  (0x01U)
#define BMS_JK_FLAG_ZERO_DATA       (0x02U)
#define BMS_JK_FLAG_NEEDS_RETRY     (0x04U)
#define BMS_JK_FLAG_USE_LAST_GOOD   (0x08U)
#define BMS_JK_FLAG_COMMAND_LOCKED  (0x10U)

/* ---------------------------------------------------------------------------
   CAN export IDs
   --------------------------------------------------------------------------- */
#define BMS_JK_CAN_ID_PACK_INFO     (0x140U)
#define BMS_JK_CAN_ID_CELL_VOLTS_1_8 (0x141U)
#define BMS_JK_CAN_ID_CELL_VOLTS_9_16 (0x142U)
#define BMS_JK_CAN_ID_TEMP_SUMMARY  (0x143U)
#define BMS_JK_CAN_ID_ALARM_STATUS  (0x144U)
#define BMS_JK_CAN_ID_CYCLE_STATS   (0x145U)

#define BMS_JK_MAX_FRAME_LEN        (32U)
#define BMS_JK_MAX_RX_BYTES         (64U)
#define BMS_JK_MAX_CELL_COUNT       (16U)
#define BMS_JK_MAX_TEMP_SENSORS     (8U)

/* ---------------------------------------------------------------------------
   JK telemetry snapshot
   --------------------------------------------------------------------------- */
typedef struct{
    int32_t packVoltageMV;      /* pack voltage in mV */
    int32_t packCurrentMA;      /* signed current in mA */
    uint8_t soc;               /* 0..100 */
    uint8_t soh;               /* 0..100 */
    uint8_t statusFlags;       /* bitfield */
    uint8_t modeFlags;         /* mode / protection bits */

    uint16_t cellVoltageMV[BMS_JK_MAX_CELL_COUNT];
    uint8_t cellCount;

    int16_t cellTemperatureC[BMS_JK_MAX_TEMP_SENSORS];
    int16_t mosTemperatureC;
    int16_t ambientTemperatureC;
    uint8_t tempSensorCount;

    uint8_t alarmFlags;
    uint8_t balanceActive;
    uint8_t protectionFlag;

    uint16_t cycleCount;
    uint32_t remainingCapacityMah;
    uint32_t fullCapacityMah;
    uint8_t resistanceEstimate;
} BMS_JK_SnapshotTypeDef;

/* ---------------------------------------------------------------------------
   JK module handle
   --------------------------------------------------------------------------- */
typedef struct{
    UART_HandleTypeDef *huart;
    GPIO_TypeDef *rs485_dir_port;
    uint16_t rs485_dir_pin;
    GPIO_TypeDef *rs485_re_port;
    uint16_t rs485_re_pin;

    uint8_t rxBuffer[BMS_JK_MAX_RX_BYTES];
    volatile uint16_t rxLen;
    uint8_t txBuffer[BMS_JK_MAX_FRAME_LEN];
    volatile uint16_t txLen;

    BMS_JK_SnapshotTypeDef snapshot;
    volatile uint8_t initialized;
    volatile uint8_t lastGoodCommand;
    volatile uint8_t lastReadCommand;
    volatile uint8_t pollIndex;
    volatile uint8_t lastReadValid;
    volatile uint8_t validationFlags;
    volatile uint8_t retryCount;
} BMS_JK_HandleTypeDef;

/* ---------------------------------------------------------------------------
   Public API
   --------------------------------------------------------------------------- */
void BMS_JK_Init(BMS_JK_HandleTypeDef *jk,
                UART_HandleTypeDef *huart,
                GPIO_TypeDef *dirPort,
                uint16_t dirPin,
                GPIO_TypeDef *rePort,
                uint16_t rePin);

void BMS_JK_SetTxMode(BMS_JK_HandleTypeDef *jk, uint8_t enabled);
void BMS_JK_SetRxMode(BMS_JK_HandleTypeDef *jk, uint8_t enabled);

HAL_StatusTypeDef BMS_JK_SendRequest(BMS_JK_HandleTypeDef *jk, uint8_t command, uint32_t timeout);
HAL_StatusTypeDef BMS_JK_ReceiveResponse(BMS_JK_HandleTypeDef *jk, uint32_t timeout);

void BMS_JK_EnterCriticalSection(void);
void BMS_JK_ExitCriticalSection(void);
uint8_t BMS_JK_ValidateResponseFrame(const uint8_t *data, uint16_t len, uint8_t expectedCommand);
HAL_StatusTypeDef BMS_JK_DecodeFrame(BMS_JK_HandleTypeDef *jk, const uint8_t *data, uint16_t len);
uint8_t BMS_JK_GetNextPollCommand(BMS_JK_HandleTypeDef *jk);
uint8_t BMS_JK_IsSnapshotValidForCommand(const BMS_JK_SnapshotTypeDef *snapshot, uint8_t command);
void BMS_JK_UpdateCommandState(BMS_JK_HandleTypeDef *jk, uint8_t command, uint8_t isValid);
void BMS_JK_UpdateValidationFlags(BMS_JK_HandleTypeDef *jk, uint8_t sentCommand, uint8_t responseValid, uint8_t payloadZero);
uint8_t BMS_JK_ShouldRetryCommand(const BMS_JK_HandleTypeDef *jk);
uint8_t BMS_JK_ApplyReadoutDecision(BMS_JK_HandleTypeDef *jk,
                                    uint8_t sentCommand,
                                    uint8_t responseOk,
                                    uint8_t payloadZero,
                                    uint8_t decodeOk,
                                    uint8_t snapshotOk);
void BMS_JK_ClearSnapshot(BMS_JK_HandleTypeDef *jk);

HAL_StatusTypeDef BMS_JK_ExportPackInfoCAN(CAN_HandleTypeDef *hcan, const BMS_JK_SnapshotTypeDef *snapshot);
HAL_StatusTypeDef BMS_JK_ExportCellVoltagesCAN(CAN_HandleTypeDef *hcan, const BMS_JK_SnapshotTypeDef *snapshot);
HAL_StatusTypeDef BMS_JK_ExportTemperatureSummaryCAN(CAN_HandleTypeDef *hcan, const BMS_JK_SnapshotTypeDef *snapshot);
HAL_StatusTypeDef BMS_JK_ExportAlarmStatusCAN(CAN_HandleTypeDef *hcan, const BMS_JK_SnapshotTypeDef *snapshot);
HAL_StatusTypeDef BMS_JK_ExportCycleStatsCAN(CAN_HandleTypeDef *hcan, const BMS_JK_SnapshotTypeDef *snapshot);

#ifdef __cplusplus
}
#endif

#endif /* BMS_JK_H_ */
