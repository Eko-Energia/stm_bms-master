/**
 * @file bms_pwm_controller.hpp
 * @brief Host-testable relay PWM: startup 100% → operational 50% after 2 s.
 */
#pragma once

#include "bms_fw_constants.hpp"
#include "hal_status.hpp"
#include "ipwm_timer.hpp"
#include "stm32_hal_stub.hpp"

#include <cmath>
#include <cstdint>

namespace bms::logic {

enum class PwmMode : uint8_t { Startup = 0, Operational = 1 };

constexpr uint32_t kPwmStartupPeriodMs = 2000;
constexpr float kRelayStartupDuty = 100.f;
constexpr float kRelayOperationalDuty = 50.f;
constexpr uint32_t kRelayTimChannel = TIM_CHANNEL_3;

class PwmController {
 public:
  explicit PwmController(mocks::IPwmTimer& tim) : tim_(tim) {}

  bms::HalStatus init(uint32_t nowMs) {
    startupStartMs_ = nowMs;
    mode_ = PwmMode::Startup;
    const auto st = tim_.start(kRelayTimChannel);
    if (st != bms::HalStatus::Ok) {
      return st;
    }
    setDuty(kRelayStartupDuty);
    return bms::HalStatus::Ok;
  }

  bms::HalStatus update(uint32_t nowMs) {
    if (mode_ == PwmMode::Startup && (nowMs - startupStartMs_) >= kPwmStartupPeriodMs) {
      mode_ = PwmMode::Operational;
    }
    setDuty(mode_ == PwmMode::Startup ? kRelayStartupDuty : kRelayOperationalDuty);
    return bms::HalStatus::Ok;
  }

  PwmMode mode() const { return mode_; }

 private:
  void setDuty(float dutyPercent) {
    const uint32_t arr = tim_.autoReload();
    const uint32_t pulse =
        static_cast<uint32_t>(std::lround(static_cast<float>(arr) * (dutyPercent / 100.f)));
    tim_.setCompare(kRelayTimChannel, pulse);
  }

  mocks::IPwmTimer& tim_;
  PwmMode mode_{PwmMode::Startup};
  uint32_t startupStartMs_{0};
};

}  // namespace bms::logic
