#pragma once

#include "stm32_hal_stub.hpp"

namespace bms::mocks {

/** Abstract single-pin GPIO used by FanController (stub for HAL_GPIO_WritePin). */
class IGpioPort {
 public:
  virtual ~IGpioPort() = default;
  virtual void write(GPIO_PinState state) = 0;
  virtual GPIO_PinState read() const = 0;
};

}  // namespace bms::mocks
