#pragma once

#include "gmock/gmock.h"
#include "iadc_dma.hpp"

namespace bms::mocks {

class MockAdcDma : public IAdcDma {
 public:
  MOCK_METHOD(bms::HalStatus, startCircular, (uint16_t* dest, uint32_t length), (override));
  MOCK_METHOD(bms::HalStatus, stop, (), (override));
  MOCK_METHOD(void, writeSample, (uint8_t rankIndex, uint16_t raw), (override));
  MOCK_METHOD(uint16_t, readSample, (uint8_t rankIndex), (const, override));
  MOCK_METHOD(bool, isRunning, (), (const, override));
};

}  // namespace bms::mocks
