#pragma once

#include "gmock/gmock.h"
#include "idual_can.hpp"

namespace bms::mocks {

class MockDualCan : public IDualCan {
 public:
  MOCK_METHOD(bms::HalStatus, startCan1, (), (override));
  MOCK_METHOD(bms::HalStatus, startCan2, (), (override));
  MOCK_METHOD(bms::HalStatus, sendCan1, (uint32_t stdId, const uint8_t* data, uint8_t dlc), (override));
  MOCK_METHOD(void, injectCan2, (uint32_t stdId, const uint8_t* data, uint8_t dlc), (override));
  MOCK_METHOD(std::optional<CanFrame>, popCan2Rx, (), (override));
};

}  // namespace bms::mocks
