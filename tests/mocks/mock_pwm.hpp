#pragma once

#include "gmock/gmock.h"
#include "ipwm_timer.hpp"

namespace bms::mocks {

class MockPwmTimer : public IPwmTimer {
 public:
  MOCK_METHOD(bms::HalStatus, start, (uint32_t channel), (override));
  MOCK_METHOD(bms::HalStatus, stop, (uint32_t channel), (override));
  MOCK_METHOD(void, setCompare, (uint32_t channel, uint32_t pulse), (override));
  MOCK_METHOD(uint32_t, autoReload, (), (const, override));
  MOCK_METHOD(bool, isRunning, (uint32_t channel), (const, override));
};

}  // namespace bms::mocks
