#pragma once

#include "stm32_hal_stub.hpp"

#include <cstdint>

namespace bms::mocks {

/** Timer PWM output used by relay driver (stub for TIM3 CH3). */
class IPwmTimer {
 public:
  virtual ~IPwmTimer() = default;
  virtual bms::HalStatus start(uint32_t channel) = 0;
  virtual bms::HalStatus stop(uint32_t channel) = 0;
  virtual void setCompare(uint32_t channel, uint32_t pulse) = 0;
  virtual uint32_t autoReload() const = 0;
  virtual bool isRunning(uint32_t channel) const = 0;
};

}  // namespace bms::mocks
