/**
 * @file bms_fw_constants.hpp
 * @brief Host-side mirror of BMS_CAN_driver.h / BMS_Types.h constants for unit tests.
 */
#pragma once

#include <cstdint>

namespace bms::fw {

constexpr uint32_t kVoltCurrTempId = 130;
constexpr uint32_t kTherm1Id = 131;
constexpr uint32_t kTherm9Id = 139;
constexpr uint8_t kVoltCurrTempDlc = 6;
constexpr uint8_t kThermGroupDlc = 7;

constexpr float kVoltageOffset = 0.0f;
constexpr float kVoltageGain = 0.1f;
constexpr float kCurrentOffset = 300.0f;
constexpr float kCurrentGain = 0.1f;
constexpr float kTemperatureOffset = 0.0f;
constexpr float kTemperatureGain = 0.01f;

constexpr uint32_t kSafeStateId = 1;
constexpr uint8_t kSafeStateOk = 0x00;
constexpr uint8_t kSafeStateError = 0x01;

constexpr float kThermTemperatureGain = 0.39216f;
constexpr int kThermIdBase = 200;
constexpr int kThermPcbCount = 7;
constexpr int kThermPerPcb = 9;
/** Must match BMS_CAN_driver.h BMS_THERM_TOTAL */
constexpr int kThermTotalForFanLatch = 40;

constexpr float kPreCoolingTempC = 50.0f;
constexpr float kPostCoolingTempC = 40.0f;

}  // namespace bms::fw
