#pragma once

#include "gmock/gmock.h"
#include "igpio_port.hpp"

namespace bms::mocks {

class MockGpioPort : public IGpioPort {
 public:
  MOCK_METHOD(void, write, (GPIO_PinState state), (override));
  MOCK_METHOD(GPIO_PinState, read, (), (const, override));
};

}  // namespace bms::mocks
