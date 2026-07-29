/**
 * @file stm32_hal_stub.hpp
 * @brief Minimal STM32 HAL type stubs for host unit tests (not linked to Cube HAL).
 */
#pragma once

#include "hal_status.hpp"

#include <array>
#include <cstdint>

using HAL_StatusTypeDef = bms::HalStatus;
constexpr auto HAL_OK = bms::HalStatus::Ok;
constexpr auto HAL_ERROR = bms::HalStatus::Error;

enum GPIO_PinState : int {
  GPIO_PIN_RESET = 0,
  GPIO_PIN_SET = 1,
};

constexpr uint32_t TIM_CHANNEL_1 = 0x00000000U;
constexpr uint32_t TIM_CHANNEL_2 = 0x00000004U;
constexpr uint32_t TIM_CHANNEL_3 = 0x00000008U;
constexpr uint32_t TIM_CHANNEL_4 = 0x0000000CU;

struct GPIO_TypeDef {
  uint32_t stub{};
};

struct TIM_HandleTypeDef {
  uint32_t InstanceStub{};
  uint32_t arr{999};   // auto-reload (Period)
  uint32_t psc{71};    // prescaler
  std::array<uint32_t, 4> ccr{};  // compare registers CH1..CH4
  bool pwmRunning{false};
};

struct CAN_HandleTypeDef {
  uint32_t InstanceStub{};
  bool started{false};
  bool rxNotifyActive{false};
};

struct ADC_HandleTypeDef {
  uint32_t InstanceStub{};
  bool converting{false};
  uint32_t resolution{4096};  // 12-bit F1
};

struct UART_HandleTypeDef {
  uint32_t InstanceStub{};
};

struct DMA_HandleTypeDef {
  uint32_t InstanceStub{};
  bool circular{true};
  bool enabled{false};
  uint16_t* dest{nullptr};
  uint32_t length{0};
};

struct CAN_TxHeaderTypeDef {
  uint32_t StdId{0};
  uint32_t ExtId{0};
  uint32_t IDE{0};
  uint32_t RTR{0};
  uint32_t DLC{0};
};

struct CAN_RxHeaderTypeDef {
  uint32_t StdId{0};
  uint32_t ExtId{0};
  uint32_t IDE{0};
  uint32_t RTR{0};
  uint32_t DLC{0};
};
