#pragma once

#include "iadc_dma.hpp"

#include <cstring>

namespace bms::mocks {

/**
 * Stub: DMA circular buffer of 3 half-words (rank0=TEMP, rank1=HALL, rank2=VOLTAGE).
 */
class StubAdcDma : public IAdcDma {
 public:
  bms::HalStatus startCircular(uint16_t* dest, uint32_t length) override {
    if (dest == nullptr || length < static_cast<uint32_t>(kAdcChannelCount)) {
      return bms::HalStatus::Error;
    }
    dest_ = dest;
    length_ = length;
    running_ = true;
    std::memset(dest_, 0, sizeof(uint16_t) * length_);
    return bms::HalStatus::Ok;
  }

  bms::HalStatus stop() override {
    running_ = false;
    return bms::HalStatus::Ok;
  }

  void writeSample(uint8_t rankIndex, uint16_t raw) override {
    if (!running_ || dest_ == nullptr || rankIndex >= kAdcChannelCount) {
      return;
    }
    dest_[rankIndex] = raw;
  }

  uint16_t readSample(uint8_t rankIndex) const override {
    if (dest_ == nullptr || rankIndex >= kAdcChannelCount) {
      return 0;
    }
    return dest_[rankIndex];
  }

  bool isRunning() const override { return running_; }

 private:
  uint16_t* dest_{nullptr};
  uint32_t length_{0};
  bool running_{false};
};

}  // namespace bms::mocks
