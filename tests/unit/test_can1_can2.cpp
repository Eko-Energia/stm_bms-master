#include "bms_can1_scheduler.hpp"
#include "bms_can2_filter.hpp"
#include "bms_can_rx_processor.hpp"
#include "bms_fw_constants.hpp"
#include "bms_protocol.hpp"
#include "idual_can.hpp"
#include "gtest/gtest.h"

using namespace bms::logic;
using namespace bms::fw;
using namespace bms::mocks;
using namespace bms::protocol;

TEST(Can2Filter, AcceptsSafeStateAndThermRange) {
  EXPECT_TRUE(can2FilterAccepts(kSafeStateId));
  EXPECT_TRUE(can2FilterAccepts(211));
  EXPECT_TRUE(can2FilterAccepts(279));
  EXPECT_FALSE(can2FilterAccepts(130));
  EXPECT_FALSE(can2FilterAccepts(200));  // pcb=0 invalid
  EXPECT_FALSE(can2FilterAccepts(210));  // therm=0 invalid
}

TEST(Can2Stub, InjectAndPopThroughFilter) {
  StubDualCan bus;
  ASSERT_EQ(bus.startCan2(), bms::HalStatus::Ok);

  const uint8_t ok = kSafeStateOk;
  bus.injectCan2(kSafeStateId, &ok, 1);

  uint8_t therm = 42;
  bus.injectCan2(thermStdId(1, 1), &therm, 1);
  bus.injectCan2(0x130, &therm, 1);  // would be HW-filtered on real device

  CanRxProcessor rx;
  while (auto frame = bus.popCan2Rx()) {
    if (!can2FilterAccepts(frame->stdId)) {
      continue;
    }
    rx.handleFrame(frame->stdId, frame->data.at(0));
  }

  EXPECT_EQ(rx.safeStateStatus(), kSafeStateOk);
  EXPECT_EQ(rx.cell(1, 1), 42);
}

TEST(Can1Scheduler, EmitsDueFramesOnPeriod) {
  StubDualCan bus;
  ASSERT_EQ(bus.startCan1(), bms::HalStatus::Ok);

  Can1Scheduler sched(bus);
  uint16_t counter = 0;
  ScheduledTx msg;
  msg.stdId = kVoltCurrTempId;
  msg.dlc = kVoltCurrTempDlc;
  msg.periodMs = 500;
  msg.lastTick = 0;
  msg.getData = [&](uint8_t* data) {
    data[0] = static_cast<uint8_t>(counter & 0xFF);
    data[1] = static_cast<uint8_t>((counter >> 8) & 0xFF);
    ++counter;
  };
  ASSERT_EQ(sched.add(msg), bms::HalStatus::Ok);

  sched.handle(100);
  EXPECT_TRUE(bus.can1Tx.empty());

  sched.handle(500);
  ASSERT_EQ(bus.can1Tx.size(), 1u);
  EXPECT_EQ(bus.can1Tx[0].stdId, kVoltCurrTempId);
  EXPECT_EQ(bus.can1Tx[0].data[0], 0);

  sched.handle(1000);
  ASSERT_EQ(bus.can1Tx.size(), 2u);
  EXPECT_EQ(bus.can1Tx[1].data[0], 1);
}

TEST(Can1Scheduler, RejectsDuplicateId) {
  StubDualCan bus;
  Can1Scheduler sched(bus);
  ScheduledTx msg;
  msg.stdId = 130;
  msg.dlc = 6;
  msg.periodMs = 500;
  msg.getData = [](uint8_t*) {};
  ASSERT_EQ(sched.add(msg), bms::HalStatus::Ok);
  EXPECT_EQ(sched.add(msg), bms::HalStatus::Error);
}
