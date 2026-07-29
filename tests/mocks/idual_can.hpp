#pragma once

#include "hal_status.hpp"
#include "ican_bus.hpp"

#include <cstdint>
#include <deque>
#include <optional>
#include <vector>

namespace bms::mocks {

/** Dual-CAN view used by Master: CAN1 = host TX, CAN2 = slave RX. */
class IDualCan {
 public:
  virtual ~IDualCan() = default;
  virtual bms::HalStatus startCan1() = 0;
  virtual bms::HalStatus startCan2() = 0;
  virtual bms::HalStatus sendCan1(uint32_t stdId, const uint8_t* data, uint8_t dlc) = 0;
  virtual void injectCan2(uint32_t stdId, const uint8_t* data, uint8_t dlc) = 0;
  virtual std::optional<CanFrame> popCan2Rx() = 0;
};

class StubDualCan : public IDualCan {
 public:
  bms::HalStatus startCan1() override {
    can1Started = true;
    return bms::HalStatus::Ok;
  }

  bms::HalStatus startCan2() override {
    can2Started = true;
    return bms::HalStatus::Ok;
  }

  bms::HalStatus sendCan1(uint32_t stdId, const uint8_t* data, uint8_t dlc) override {
    if (!can1Started) {
      return bms::HalStatus::Error;
    }
    CanFrame f;
    f.stdId = stdId;
    f.data.assign(data, data + dlc);
    can1Tx.push_back(std::move(f));
    return bms::HalStatus::Ok;
  }

  void injectCan2(uint32_t stdId, const uint8_t* data, uint8_t dlc) override {
    CanFrame f;
    f.stdId = stdId;
    f.data.assign(data, data + dlc);
    can2Rx.push_back(std::move(f));
  }

  std::optional<CanFrame> popCan2Rx() override {
    if (can2Rx.empty()) {
      return std::nullopt;
    }
    CanFrame f = std::move(can2Rx.front());
    can2Rx.pop_front();
    return f;
  }

  bool can1Started{false};
  bool can2Started{false};
  std::vector<CanFrame> can1Tx;
  std::deque<CanFrame> can2Rx;
};

}  // namespace bms::mocks
