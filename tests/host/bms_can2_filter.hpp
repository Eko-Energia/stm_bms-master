/**
 * @file bms_can2_filter.hpp
 * @brief Host mirror of CAN2 HW filters (SAFE_STATE + therm 0x200..0x27F).
 */
#pragma once

#include "bms_fw_constants.hpp"

#include <cstdint>

namespace bms::logic {

inline bool can2FilterAccepts(uint32_t stdId) {
  if (stdId == fw::kSafeStateId) {
    return true;
  }
  /* Therm StdId = 200 + pcb*10 + therm (decimal), pcb 1..7, therm 1..9 → 211..279 */
  if (stdId < static_cast<uint32_t>(fw::kThermIdBase)) {
    return false;
  }
  const int pcb = (static_cast<int>(stdId) - fw::kThermIdBase) / 10;
  const int therm = static_cast<int>(stdId) - fw::kThermIdBase - pcb * 10;
  return pcb >= 1 && pcb <= fw::kThermPcbCount && therm >= 1 && therm <= fw::kThermPerPcb;
}

}  // namespace bms::logic
