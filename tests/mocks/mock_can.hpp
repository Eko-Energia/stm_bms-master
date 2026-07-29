#pragma once

#include "gmock/gmock.h"
#include "ican_bus.hpp"

namespace bms::mocks {

class MockCanBus : public ICanBus {
 public:
  MOCK_METHOD(void, send, (uint32_t stdId, const uint8_t* data, uint8_t dlc), (override));
};

}  // namespace bms::mocks
