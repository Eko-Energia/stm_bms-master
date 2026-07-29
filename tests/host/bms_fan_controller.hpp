/**
 * @file bms_fan_controller.hpp
 * @brief Host-testable mirror of BMS_FAN_Control hysteresis.
 */
#pragma once

#include "bms_fw_constants.hpp"
#include "igpio_port.hpp"
#include "hal_status.hpp"

#include <cstdint>

namespace bms::logic {

enum class FanState : uint8_t { Off = 0, On = 1 };

class FanController {
 public:
  explicit FanController(mocks::IGpioPort& fanPin) : fanPin_(fanPin) {}

  bms::HalStatus update(float maxTemperatureC);

  FanState state() const { return state_; }

 private:
  mocks::IGpioPort& fanPin_;
  FanState state_{FanState::Off};
};

}  // namespace bms::logic
