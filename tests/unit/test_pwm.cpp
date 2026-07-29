#include "bms_pwm_controller.hpp"
#include "mock_pwm.hpp"
#include "stub_pwm.hpp"
#include "stm32_hal_stub.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace bms::logic;
using namespace bms::mocks;
using ::testing::_;
using ::testing::Return;

TEST(PwmStub, StartupThenOperationalDuty) {
  TIM_HandleTypeDef tim{};
  tim.arr = 999;
  StubPwmTimer stub(tim);
  PwmController pwm(stub);

  ASSERT_EQ(pwm.init(0), bms::HalStatus::Ok);
  EXPECT_TRUE(stub.isRunning(kRelayTimChannel));
  EXPECT_NEAR(stub.dutyPercent(kRelayTimChannel), 100.f, 0.6f);
  EXPECT_EQ(pwm.mode(), PwmMode::Startup);

  ASSERT_EQ(pwm.update(1999), bms::HalStatus::Ok);
  EXPECT_EQ(pwm.mode(), PwmMode::Startup);
  EXPECT_NEAR(stub.dutyPercent(kRelayTimChannel), 100.f, 0.6f);

  ASSERT_EQ(pwm.update(2000), bms::HalStatus::Ok);
  EXPECT_EQ(pwm.mode(), PwmMode::Operational);
  EXPECT_NEAR(stub.dutyPercent(kRelayTimChannel), 50.f, 0.6f);
}

TEST(PwmMock, StartAndSetCompareCalled) {
  MockPwmTimer mock;
  PwmController pwm(mock);

  EXPECT_CALL(mock, start(TIM_CHANNEL_3)).WillOnce(Return(bms::HalStatus::Ok));
  EXPECT_CALL(mock, autoReload()).WillRepeatedly(Return(999u));
  EXPECT_CALL(mock, setCompare(TIM_CHANNEL_3, _)).Times(::testing::AtLeast(1));

  ASSERT_EQ(pwm.init(0), bms::HalStatus::Ok);
  ASSERT_EQ(pwm.update(2500), bms::HalStatus::Ok);
}
