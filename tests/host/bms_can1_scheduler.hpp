/**
 * @file bms_can1_scheduler.hpp
 * @brief Host-testable mirror of CAN1 scheduled TX (period + getData).
 */
#pragma once

#include "bms_fw_constants.hpp"
#include "hal_status.hpp"
#include "idual_can.hpp"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <vector>

namespace bms::logic {

struct ScheduledTx {
  uint32_t stdId{0};
  uint8_t dlc{0};
  uint32_t periodMs{0};
  uint32_t lastTick{0};
  std::function<void(uint8_t* data)> getData;
};

class Can1Scheduler {
 public:
  explicit Can1Scheduler(mocks::IDualCan& can) : can_(can) {}

  bms::HalStatus add(ScheduledTx msg) {
    if (msg.periodMs == 0 || !msg.getData) {
      return bms::HalStatus::Error;
    }
    for (const auto& existing : list_) {
      if (existing.stdId == msg.stdId) {
        return bms::HalStatus::Error;
      }
    }
    list_.push_back(std::move(msg));
    return bms::HalStatus::Ok;
  }

  /** Process due frames at currentTick (ms). */
  void handle(uint32_t currentTickMs) {
    uint8_t data[8]{};
    for (auto& msg : list_) {
      if (currentTickMs >= msg.lastTick + msg.periodMs) {
        std::fill(std::begin(data), std::end(data), 0);
        msg.getData(data);
        can_.sendCan1(msg.stdId, data, msg.dlc);
        msg.lastTick = currentTickMs;
      }
    }
  }

  std::size_t size() const { return list_.size(); }

 private:
  mocks::IDualCan& can_;
  std::vector<ScheduledTx> list_;
};

}  // namespace bms::logic
