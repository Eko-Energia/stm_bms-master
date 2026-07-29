#include <cmath>

#include "bms_protocol.hpp"
#include "gtest/gtest.h"

using namespace bms::protocol;
using namespace bms::fw;

TEST(ProtocolScaling, VoltageRoundTrip) {
  const float v = 48.0f;
  const uint16_t raw = encodePhysicalToU16(v, kVoltageOffset, kVoltageGain);
  EXPECT_NEAR(decodeU16ToPhysical(raw, kVoltageOffset, kVoltageGain), v, kVoltageGain);
}

TEST(ProtocolScaling, CurrentRange) {
  for (float a : {-300.f, -150.f, 0.f, 150.f, 300.f}) {
    const uint16_t raw = encodePhysicalToU16(a, kCurrentOffset, kCurrentGain);
    EXPECT_NEAR(decodeU16ToPhysical(raw, kCurrentOffset, kCurrentGain), a, kCurrentGain);
  }
}

TEST(ProtocolScaling, TemperatureRoundTrip) {
  const float t = 25.0f;
  const uint16_t raw = encodePhysicalToU16(t, kTemperatureOffset, kTemperatureGain);
  EXPECT_NEAR(decodeU16ToPhysical(raw, kTemperatureOffset, kTemperatureGain), t, kTemperatureGain);
}

TEST(ProtocolFrame, VoltCurrTempLayout) {
  VoltCurrTemp in{87.0f, -10.0f, 30.0f};
  const auto frame = in.toFrame();
  const auto out = VoltCurrTemp::fromFrame(frame.data(), frame.size());
  EXPECT_NEAR(out.voltageV, 87.0f, kVoltageGain);
  EXPECT_NEAR(out.currentA, -10.0f, kCurrentGain);
  EXPECT_NEAR(out.temperatureC, 30.0f, kTemperatureGain);
}

TEST(ProtocolThermId, Encoding) {
  EXPECT_EQ(thermStdId(1, 1), 211u);
  EXPECT_EQ(thermStdId(7, 9), 279u);
  EXPECT_THROW(thermStdId(0, 1), std::invalid_argument);
}

TEST(ProtocolThermByte, GainEncode) {
  const uint8_t raw = tempCToThermByte(55.0f);
  EXPECT_NEAR(thermByteToTempC(raw), 55.0f, kThermTemperatureGain);
}

TEST(ProtocolThermSlots, FanLatchCount) {
  const auto slots = firstNThermSlots(kThermTotalForFanLatch);
  EXPECT_EQ(slots.size(), static_cast<std::size_t>(kThermTotalForFanLatch));
  EXPECT_EQ(allThermSlots().size(), static_cast<std::size_t>(kThermPcbCount * kThermPerPcb));
}
