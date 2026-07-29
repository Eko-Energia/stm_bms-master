#pragma once

#include "ipwm_timer.hpp"
#include "stm32_hal_stub.hpp"

namespace bms::mocks {

/** Concrete stub TIM with ARR/CCR state (mirrors HAL timer registers). */
class StubPwmTimer : public IPwmTimer {
 public:
  explicit StubPwmTimer(TIM_HandleTypeDef& tim) : tim_(tim) {}

  bms::HalStatus start(uint32_t /*channel*/) override {
    tim_.pwmRunning = true;
    return bms::HalStatus::Ok;
  }

  bms::HalStatus stop(uint32_t /*channel*/) override {
    tim_.pwmRunning = false;
    return bms::HalStatus::Ok;
  }

  void setCompare(uint32_t channel, uint32_t pulse) override {
    const int idx = channelToIndex(channel);
    if (idx >= 0) {
      tim_.ccr[static_cast<std::size_t>(idx)] = pulse;
    }
  }

  uint32_t autoReload() const override { return tim_.arr; }

  bool isRunning(uint32_t /*channel*/) const override { return tim_.pwmRunning; }

  float dutyPercent(uint32_t channel) const {
    if (tim_.arr == 0) {
      return 0.f;
    }
    const int idx = channelToIndex(channel);
    if (idx < 0) {
      return 0.f;
    }
    return 100.f * static_cast<float>(tim_.ccr[static_cast<std::size_t>(idx)]) /
           static_cast<float>(tim_.arr);
  }

 private:
  static int channelToIndex(uint32_t channel) {
    if (channel == TIM_CHANNEL_1) return 0;
    if (channel == TIM_CHANNEL_2) return 1;
    if (channel == TIM_CHANNEL_3) return 2;
    if (channel == TIM_CHANNEL_4) return 3;
    return -1;
  }

  TIM_HandleTypeDef& tim_;
};

}  // namespace bms::mocks
