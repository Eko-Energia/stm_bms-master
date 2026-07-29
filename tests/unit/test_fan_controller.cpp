#include "bms_fan_controller.hpp"
#include "mock_gpio.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace bms::logic;
using namespace bms::fw;
using namespace bms::mocks;
using ::testing::_;
using ::testing::InSequence;
using ::testing::Return;

TEST(FanController, TurnsOnAbovePreCooling) {
  MockGpioPort gpio;
  FanController fan(gpio);

  EXPECT_CALL(gpio, write(GPIO_PIN_SET)).Times(1);
  ASSERT_EQ(fan.update(kPreCoolingTempC), bms::HalStatus::Ok);
  EXPECT_EQ(fan.state(), FanState::On);
}

TEST(FanController, HysteresisHoldsBetweenThresholds) {
  MockGpioPort gpio;
  FanController fan(gpio);

  {
    InSequence seq;
    EXPECT_CALL(gpio, write(GPIO_PIN_SET)).Times(1);
    EXPECT_CALL(gpio, write(GPIO_PIN_SET)).Times(1);  // still ON at 45 C
    EXPECT_CALL(gpio, write(GPIO_PIN_RESET)).Times(1);
  }

  fan.update(55.f);
  fan.update(45.f);  // between 40 and 50 — stay ON
  EXPECT_EQ(fan.state(), FanState::On);
  fan.update(kPostCoolingTempC);
  EXPECT_EQ(fan.state(), FanState::Off);
}

TEST(FanController, StaysOffWhenCool) {
  MockGpioPort gpio;
  FanController fan(gpio);
  EXPECT_CALL(gpio, write(GPIO_PIN_RESET)).Times(1);
  fan.update(20.f);
  EXPECT_EQ(fan.state(), FanState::Off);
}
