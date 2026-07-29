/**
 * @file bms_can_rx_processor.hpp
 * @brief Host-testable mirror of BMS_CAN_HandleRxMsg therm/SAFE_STATE logic.
 */
#pragma once

#include "bms_fw_constants.hpp"
#include "hal_status.hpp"

#include <array>
#include <cstdint>

namespace bms::logic {

/**
 * Pure logic extracted for unit tests — mirrors BMS_CAN_HandleRxMsg behaviour
 * (SAFE_STATE early return, bounds check, unique scan latch).
 */
class CanRxProcessor {
 public:
  CanRxProcessor() { resetScan(); }

  void resetScan();

  /** Process one CAN2 frame (StdId + first data byte). */
  bms::HalStatus handleFrame(uint32_t stdId, uint8_t rxByte);

  uint8_t safeStateStatus() const { return safeStateStatus_; }
  float maxTemperature() const { return maxTemperature_; }
  float scanMax() const { return scanMax_; }
  uint8_t uniqueCount() const { return uniqueCount_; }

  uint8_t cell(int pcb /*1..7*/, int therm /*1..9*/) const {
    return cells_[static_cast<std::size_t>(pcb - 1)][static_cast<std::size_t>(therm - 1)];
  }

  /** Pack CAN1 therm group payload: data[i] = PCB(i+1) for this therm index 0..8 */
  void packThermGroup(uint8_t thermIndex0, uint8_t out[7]) const;

 private:
  uint8_t safeStateStatus_{fw::kSafeStateOk};
  float maxTemperature_{0.f};
  float scanMax_{-1000.f};
  uint8_t uniqueCount_{0};
  std::array<std::array<uint8_t, fw::kThermPerPcb>, fw::kThermPcbCount> cells_{};
  std::array<std::array<uint8_t, fw::kThermPerPcb>, fw::kThermPcbCount> seen_{};
};

}  // namespace bms::logic
