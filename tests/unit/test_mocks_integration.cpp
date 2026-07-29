#include <algorithm>
#include <random>

#include "bms_can_rx_processor.hpp"
#include "bms_fan_controller.hpp"
#include "bms_protocol.hpp"
#include "ican_bus.hpp"
#include "mock_can.hpp"
#include "mock_gpio.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace bms::logic;
using namespace bms::protocol;
using namespace bms::fw;
using namespace bms::mocks;
using ::testing::_;

/**
 * Integration-style unit test: CAN2 inject (stub) → RX processor → FAN mock GPIO.
 * Mirrors the bring-up path without STM32 hardware.
 */
TEST(MocksIntegration, HotScanLatchesAndTurnsFanOn) {
  MockGpioPort fanGpio;
  StubCanBus can2Inject;
  CanRxProcessor rx;
  FanController fan(fanGpio);

  auto slots = firstNThermSlots(kThermTotalForFanLatch);
  std::mt19937 rng(7);
  std::shuffle(slots.begin(), slots.end(), rng);

  const uint8_t hotByte = tempCToThermByte(60.f);
  for (const auto& [pcb, therm] : slots) {
    const uint32_t id = thermStdId(pcb, therm);
    uint8_t data[1] = {hotByte};
    can2Inject.send(id, data, 1);
  }

  ASSERT_EQ(can2Inject.tx.size(), static_cast<std::size_t>(kThermTotalForFanLatch));

  // SAFE_STATE first (must not corrupt scan)
  rx.handleFrame(kSafeStateId, kSafeStateOk);

  for (const auto& frame : can2Inject.tx) {
    rx.handleFrame(frame.stdId, frame.data.at(0));
  }

  EXPECT_NEAR(rx.maxTemperature(), thermByteToTempC(hotByte), kThermTemperatureGain);

  EXPECT_CALL(fanGpio, write(GPIO_PIN_SET)).Times(1);
  fan.update(rx.maxTemperature());
  EXPECT_EQ(fan.state(), FanState::On);

  // Republish check via packThermGroup for first slot's therm index
  const auto oneSlot = firstNThermSlots(1);
  const int pcb0 = oneSlot.front().first;
  const int therm0 = oneSlot.front().second;
  uint8_t group[7]{};
  rx.packThermGroup(static_cast<uint8_t>(therm0 - 1), group);
  EXPECT_EQ(group[pcb0 - 1], hotByte);
}

TEST(MocksIntegration, MockCanSendExpectation) {
  MockCanBus bus;
  const uint8_t payload[1] = {0x55};
  EXPECT_CALL(bus, send(kSafeStateId, _, 1)).Times(1);
  bus.send(kSafeStateId, payload, 1);
}
