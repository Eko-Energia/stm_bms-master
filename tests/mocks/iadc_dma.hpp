#pragma once

#include "hal_status.hpp"

#include <cstdint>

namespace bms::mocks {

constexpr int kAdcChannelCount = 3;  // TEMP / HALL / VOLTAGE ranks

/** Circular ADC+DMA buffer interface (ADC1 + DMA1_CH1 path). */
class IAdcDma {
 public:
  virtual ~IAdcDma() = default;
  virtual bms::HalStatus startCircular(uint16_t* dest, uint32_t length) = 0;
  virtual bms::HalStatus stop() = 0;
  virtual void writeSample(uint8_t rankIndex, uint16_t raw) = 0;
  virtual uint16_t readSample(uint8_t rankIndex) const = 0;
  virtual bool isRunning() const = 0;
};

}  // namespace bms::mocks
