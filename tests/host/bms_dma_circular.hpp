/**
 * @file bms_dma_circular.hpp
 * @brief Host stub helper for DMA1 circular half-word transfers.
 */
#pragma once

#include "hal_status.hpp"
#include "stm32_hal_stub.hpp"

#include <cstdint>
#include <cstring>

namespace bms::logic {

class DmaCircular {
 public:
  bms::HalStatus configure(DMA_HandleTypeDef& hdma, uint16_t* dest, uint32_t length) {
    if (dest == nullptr || length == 0) {
      return bms::HalStatus::Error;
    }
    hdma_ = &hdma;
    hdma_->dest = dest;
    hdma_->length = length;
    hdma_->circular = true;
    hdma_->enabled = false;
    return bms::HalStatus::Ok;
  }

  bms::HalStatus start() {
    if (hdma_ == nullptr || hdma_->dest == nullptr) {
      return bms::HalStatus::Error;
    }
    hdma_->enabled = true;
    return bms::HalStatus::Ok;
  }

  bms::HalStatus stop() {
    if (hdma_ == nullptr) {
      return bms::HalStatus::Error;
    }
    hdma_->enabled = false;
    return bms::HalStatus::Ok;
  }

  /** Simulate DMA writing one conversion set into the circular buffer. */
  void pushSamples(const uint16_t* samples, uint32_t count) {
    if (hdma_ == nullptr || !hdma_->enabled || samples == nullptr) {
      return;
    }
    const uint32_t n = (count < hdma_->length) ? count : hdma_->length;
    std::memcpy(hdma_->dest, samples, n * sizeof(uint16_t));
  }

  bool enabled() const { return hdma_ != nullptr && hdma_->enabled; }

 private:
  DMA_HandleTypeDef* hdma_{nullptr};
};

}  // namespace bms::logic
