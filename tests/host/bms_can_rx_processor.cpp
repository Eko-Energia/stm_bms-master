#include "bms_can_rx_processor.hpp"

#include "bms_protocol.hpp"

namespace bms::logic {

void CanRxProcessor::resetScan() {
  uniqueCount_ = 0;
  scanMax_ = -1000.f;
  for (auto& row : seen_) {
    row.fill(0);
  }
}

bms::HalStatus CanRxProcessor::handleFrame(uint32_t stdId, uint8_t rxByte) {
  if (stdId == fw::kSafeStateId) {
    safeStateStatus_ = rxByte;
    return bms::HalStatus::Ok;
  }

  const int pcbIndex = (static_cast<int>(stdId) - fw::kThermIdBase) / 10;
  const int thermIndex = static_cast<int>(stdId) - fw::kThermIdBase - pcbIndex * 10;

  if (pcbIndex < 1 || pcbIndex > fw::kThermPcbCount || thermIndex < 1 ||
      thermIndex > fw::kThermPerPcb) {
    return bms::HalStatus::Ok;
  }

  const float thermTemperature = protocol::thermByteToTempC(rxByte);

  cells_[static_cast<std::size_t>(pcbIndex - 1)][static_cast<std::size_t>(thermIndex - 1)] = rxByte;

  if (thermTemperature > scanMax_) {
    scanMax_ = thermTemperature;
  }

  auto& seenFlag =
      seen_[static_cast<std::size_t>(pcbIndex - 1)][static_cast<std::size_t>(thermIndex - 1)];
  if (seenFlag == 0) {
    seenFlag = 1;
    ++uniqueCount_;
  }

  if (uniqueCount_ >= fw::kThermTotalForFanLatch) {
    maxTemperature_ = scanMax_;
    resetScan();
  }

  return bms::HalStatus::Ok;
}

void CanRxProcessor::packThermGroup(uint8_t thermIndex0, uint8_t out[7]) const {
  for (int i = 0; i < fw::kThermPcbCount; ++i) {
    out[i] = cells_[static_cast<std::size_t>(i)][thermIndex0];
  }
}

}  // namespace bms::logic
