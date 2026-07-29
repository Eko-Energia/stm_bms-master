#include "bms_adc_pipeline.hpp"
#include "bms_dma_circular.hpp"
#include "mock_adc_dma.hpp"
#include "stub_adc_dma.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace bms::logic;
using namespace bms::mocks;
using namespace bms::protocol;
using namespace bms::fw;
using ::testing::_;
using ::testing::Return;

TEST(DmaCircular, ConfigureStartPushStop) {
  DMA_HandleTypeDef hdma{};
  uint16_t buf[3]{};
  DmaCircular dma;

  ASSERT_EQ(dma.configure(hdma, buf, 3), bms::HalStatus::Ok);
  ASSERT_EQ(dma.start(), bms::HalStatus::Ok);
  EXPECT_TRUE(dma.enabled());

  const uint16_t samples[3] = {100, 2108, 500};
  dma.pushSamples(samples, 3);
  EXPECT_EQ(buf[0], 100);
  EXPECT_EQ(buf[1], 2108);
  EXPECT_EQ(buf[2], 500);

  ASSERT_EQ(dma.stop(), bms::HalStatus::Ok);
  EXPECT_FALSE(dma.enabled());
}

TEST(AdcDmaStub, CircularBufferAndPhysicalScale) {
  StubAdcDma dma;
  AdcPipeline adc(dma);

  ASSERT_EQ(adc.start(), bms::HalStatus::Ok);
  EXPECT_TRUE(dma.isRunning());

  const uint16_t voltRaw = 2048;
  const uint16_t hallZero = 2108;
  adc.writeRaw(/*temp*/ 2500, hallZero, voltRaw);

  const AdcPhysical p = adc.readPhysical();
  EXPECT_NEAR(p.currentA, 0.f, 0.1f);
  const float pinV = (2048.f / 4096.f) * 3.3f;
  EXPECT_NEAR(p.voltageV, pinV * kPackDivider, 0.5f);

  uint8_t frame[6]{};
  adc.fillVoltCurrTempFrame(frame);
  const auto decoded = VoltCurrTemp::fromFrame(frame, 6);
  EXPECT_NEAR(decoded.voltageV, p.voltageV, kVoltageGain);
  EXPECT_NEAR(decoded.currentA, p.currentA, kCurrentGain);
}

TEST(AdcDmaMock, StartStopExpectations) {
  MockAdcDma mock;
  AdcPipeline adc(mock);

  EXPECT_CALL(mock, startCircular(_, 3)).WillOnce(Return(bms::HalStatus::Ok));
  EXPECT_CALL(mock, stop()).WillOnce(Return(bms::HalStatus::Ok));

  ASSERT_EQ(adc.start(), bms::HalStatus::Ok);
  ASSERT_EQ(adc.stop(), bms::HalStatus::Ok);
}

TEST(AdcDmaStub, RejectsShortBuffer) {
  StubAdcDma dma;
  uint16_t tiny[1]{};
  EXPECT_EQ(dma.startCircular(tiny, 1), bms::HalStatus::Error);
}
