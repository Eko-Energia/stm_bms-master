/**
 * @file bms_adc_pipeline.hpp
 * @brief Host-testable ADC+DMA sense → physical → CAN1 scale (no NTC calibrate API).
 */
#pragma once

#include "bms_fw_constants.hpp"
#include "bms_protocol.hpp"
#include "hal_status.hpp"
#include "iadc_dma.hpp"

#include <array>
#include <cstdint>

namespace bms::logic {

/** Rank indices in DMA circular buffer (match Cube ADC order: TEMP, HALL, VOLTAGE). */
constexpr uint8_t kRankTemp = 0;
constexpr uint8_t kRankHall = 1;
constexpr uint8_t kRankVoltage = 2;

constexpr float kPackDivider = 28.362637362637362637f;
constexpr float kStm32Vcc = 3.3f;
constexpr float kHallZeroRaw = 2108.f;
constexpr float kHallScale = 4.f;

struct AdcPhysical {
  float voltageV{0.f};
  float currentA{0.f};
  float boardTempC{0.f};
};

class AdcPipeline {
 public:
  explicit AdcPipeline(mocks::IAdcDma& dma) : dma_(dma) {}

  bms::HalStatus start() {
    return dma_.startCircular(raw_.data(), static_cast<uint32_t>(raw_.size()));
  }

  bms::HalStatus stop() { return dma_.stop(); }

  /** Inject DMA samples (as if conversion complete). */
  void writeRaw(uint16_t tempRaw, uint16_t hallRaw, uint16_t voltRaw) {
    dma_.writeSample(kRankTemp, tempRaw);
    dma_.writeSample(kRankHall, hallRaw);
    dma_.writeSample(kRankVoltage, voltRaw);
  }

  AdcPhysical readPhysical() const {
    AdcPhysical p;
    const uint16_t voltRaw = dma_.readSample(kRankVoltage);
    const uint16_t hallRaw = dma_.readSample(kRankHall);
    const float pinV = (static_cast<float>(voltRaw) / 4096.f) * kStm32Vcc;
    p.voltageV = pinV * kPackDivider;
    p.currentA = (static_cast<float>(hallRaw) - kHallZeroRaw) / kHallScale;
    // Board NTC path uses LUT in FW; for unit tests expose pin voltage only as placeholder °C scale
    // via TEMPERATURE_GAIN encoding of a synthetic value from raw (host stub).
    p.boardTempC = static_cast<float>(dma_.readSample(kRankTemp)) * 0.01f;
    return p;
  }

  /** Fill CAN1 VOLTCURTEMP payload bytes (LE u16 V/I/T scaled). */
  void fillVoltCurrTempFrame(uint8_t out[6]) const {
    const AdcPhysical p = readPhysical();
    protocol::VoltCurrTemp frame{p.voltageV, p.currentA, p.boardTempC};
    const auto bytes = frame.toFrame();
    for (int i = 0; i < 6; ++i) {
      out[i] = bytes[static_cast<std::size_t>(i)];
    }
  }

  const std::array<uint16_t, 3>& rawBuffer() const { return raw_; }

 private:
  mocks::IAdcDma& dma_;
  std::array<uint16_t, 3> raw_{};
};

}  // namespace bms::logic
