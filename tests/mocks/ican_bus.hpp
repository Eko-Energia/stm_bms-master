#pragma once

#include <cstdint>
#include <vector>

namespace bms::mocks {

struct CanFrame {
  uint32_t stdId{0};
  std::vector<uint8_t> data;
};

/** Abstract CAN bus for injecting slave frames / capturing master TX in tests. */
class ICanBus {
 public:
  virtual ~ICanBus() = default;
  virtual void send(uint32_t stdId, const uint8_t* data, uint8_t dlc) = 0;
};

/** Simple stub that records TX frames for assertions without gmock matchers. */
class StubCanBus : public ICanBus {
 public:
  void send(uint32_t stdId, const uint8_t* data, uint8_t dlc) override {
    CanFrame f;
    f.stdId = stdId;
    f.data.assign(data, data + dlc);
    tx.push_back(std::move(f));
  }

  std::vector<CanFrame> tx;
};

}  // namespace bms::mocks
