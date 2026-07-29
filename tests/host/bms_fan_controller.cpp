#include "bms_fan_controller.hpp"

namespace bms::logic {

bms::HalStatus FanController::update(float maxTemperatureC) {
  if (maxTemperatureC >= fw::kPreCoolingTempC && state_ == FanState::Off) {
    state_ = FanState::On;
  } else if (maxTemperatureC <= fw::kPostCoolingTempC && state_ == FanState::On) {
    state_ = FanState::Off;
  }

  fanPin_.write(state_ == FanState::On ? GPIO_PIN_SET : GPIO_PIN_RESET);
  return bms::HalStatus::Ok;
}

}  // namespace bms::logic
