#include <algorithm>
#include <random>
#include <vector>

#include "bms_can_rx_processor.hpp"
#include "bms_protocol.hpp"
#include "gtest/gtest.h"

using namespace bms::logic;
using namespace bms::protocol;
using namespace bms::fw;

TEST(CanRxProcessor, SafeStateDoesNotTouchCells) {
  CanRxProcessor rx;
  ASSERT_EQ(rx.handleFrame(kSafeStateId, kSafeStateError), bms::HalStatus::Ok);
  EXPECT_EQ(rx.safeStateStatus(), kSafeStateError);
  EXPECT_EQ(rx.uniqueCount(), 0);
  EXPECT_EQ(rx.maxTemperature(), 0.f);
}

TEST(CanRxProcessor, IgnoresOutOfRangeIds) {
  CanRxProcessor rx;
  // ID 201 -> pcb=0 therm=1 — invalid
  EXPECT_EQ(rx.handleFrame(201, 10), bms::HalStatus::Ok);
  EXPECT_EQ(rx.uniqueCount(), 0);
}

TEST(CanRxProcessor, StoresCellAndCountsUnique) {
  CanRxProcessor rx;
  const uint32_t id = thermStdId(2, 3);
  const uint8_t raw = 120;
  ASSERT_EQ(rx.handleFrame(id, raw), bms::HalStatus::Ok);
  EXPECT_EQ(rx.cell(2, 3), raw);
  EXPECT_EQ(rx.uniqueCount(), 1);
  // Duplicate must not increase unique count
  ASSERT_EQ(rx.handleFrame(id, raw), bms::HalStatus::Ok);
  EXPECT_EQ(rx.uniqueCount(), 1);
}

TEST(CanRxProcessor, LatchesMaxAfterThermTotalUniqueInRandomOrder) {
  CanRxProcessor rx;
  auto slots = firstNThermSlots(kThermTotalForFanLatch);
  std::mt19937 rng(42);
  std::shuffle(slots.begin(), slots.end(), rng);

  const uint8_t hot = tempCToThermByte(kPreCoolingTempC + 10.f);
  for (const auto& [pcb, therm] : slots) {
    rx.handleFrame(thermStdId(pcb, therm), hot);
  }

  EXPECT_EQ(rx.uniqueCount(), 0);  // scan reset after latch
  EXPECT_NEAR(rx.maxTemperature(), thermByteToTempC(hot), kThermTemperatureGain);
}

TEST(CanRxProcessor, PackThermGroupEcho) {
  CanRxProcessor rx;
  rx.handleFrame(thermStdId(1, 1), 11);
  rx.handleFrame(thermStdId(7, 1), 77);
  uint8_t out[7]{};
  rx.packThermGroup(0, out);  // therm index 0 = therm #1
  EXPECT_EQ(out[0], 11);
  EXPECT_EQ(out[6], 77);
}
