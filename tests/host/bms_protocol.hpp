/**
 * @file bms_protocol.hpp
 * @brief Protocol encode/decode helpers matching BMS-Master CAN firmware.
 */
#pragma once

#include "bms_fw_constants.hpp"

#include <array>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>

namespace bms::protocol {

inline uint32_t thermStdId(int pcb, int therm) {
  if (pcb < 1 || pcb > fw::kThermPcbCount || therm < 1 || therm > fw::kThermPerPcb) {
    throw std::invalid_argument("invalid pcb/therm");
  }
  return static_cast<uint32_t>(fw::kThermIdBase + pcb * 10 + therm);
}

inline uint8_t tempCToThermByte(float tempC) {
  const int raw = static_cast<int>(std::lround(tempC / fw::kThermTemperatureGain));
  if (raw < 0) {
    return 0;
  }
  if (raw > 255) {
    return 255;
  }
  return static_cast<uint8_t>(raw);
}

inline float thermByteToTempC(uint8_t raw) {
  return static_cast<float>(raw) * fw::kThermTemperatureGain;
}

inline uint16_t encodePhysicalToU16(float value, float offset, float gain) {
  const float scaled = (value + offset) / gain;
  const long rounded = std::lround(scaled);
  return static_cast<uint16_t>(rounded & 0xFFFF);
}

inline float decodeU16ToPhysical(uint16_t raw, float offset, float gain) {
  return static_cast<float>(raw) * gain - offset;
}

inline void packU16Le(uint16_t value, uint8_t out[2]) {
  out[0] = static_cast<uint8_t>(value & 0xFF);
  out[1] = static_cast<uint8_t>((value >> 8) & 0xFF);
}

inline uint16_t unpackU16Le(const uint8_t* data) {
  return static_cast<uint16_t>(data[0] | (static_cast<uint16_t>(data[1]) << 8));
}

struct VoltCurrTemp {
  float voltageV{0.f};
  float currentA{0.f};
  float temperatureC{0.f};

  static VoltCurrTemp fromFrame(const uint8_t* data, std::size_t len) {
    if (len < fw::kVoltCurrTempDlc) {
      throw std::invalid_argument("VOLTCURTEMP DLC too short");
    }
    VoltCurrTemp v;
    v.voltageV = decodeU16ToPhysical(unpackU16Le(data + 0), fw::kVoltageOffset, fw::kVoltageGain);
    v.currentA = decodeU16ToPhysical(unpackU16Le(data + 2), fw::kCurrentOffset, fw::kCurrentGain);
    v.temperatureC =
        decodeU16ToPhysical(unpackU16Le(data + 4), fw::kTemperatureOffset, fw::kTemperatureGain);
    return v;
  }

  std::array<uint8_t, fw::kVoltCurrTempDlc> toFrame() const {
    std::array<uint8_t, fw::kVoltCurrTempDlc> out{};
    packU16Le(encodePhysicalToU16(voltageV, fw::kVoltageOffset, fw::kVoltageGain), out.data() + 0);
    packU16Le(encodePhysicalToU16(currentA, fw::kCurrentOffset, fw::kCurrentGain), out.data() + 2);
    packU16Le(encodePhysicalToU16(temperatureC, fw::kTemperatureOffset, fw::kTemperatureGain),
              out.data() + 4);
    return out;
  }
};

inline std::vector<std::pair<int, int>> allThermSlots() {
  std::vector<std::pair<int, int>> slots;
  slots.reserve(fw::kThermPcbCount * fw::kThermPerPcb);
  for (int pcb = 1; pcb <= fw::kThermPcbCount; ++pcb) {
    for (int therm = 1; therm <= fw::kThermPerPcb; ++therm) {
      slots.emplace_back(pcb, therm);
    }
  }
  return slots;
}

inline std::vector<std::pair<int, int>> firstNThermSlots(int n) {
  auto slots = allThermSlots();
  if (n < 0 || static_cast<std::size_t>(n) > slots.size()) {
    throw std::invalid_argument("n out of range");
  }
  slots.resize(static_cast<std::size_t>(n));
  return slots;
}

}  // namespace bms::protocol
