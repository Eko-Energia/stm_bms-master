# BMS Master

**Firmware version:** `2.0.1`  
**Hardware / project tree:** `1.19.0`  
**MCU:** STM32F105R8Tx (LQFP64)  
**Organization:** AGH Eko-Energy

Bare-metal firmware for the BMS Master PCB. Application logic lives in `BMS_Driver/`; shared peripheral drivers live in `EKO_Drivers/`.

---

## Warning

> **HIGH VOLTAGE / HVIL**  
> This firmware controls pack sensing, relay PWM, HVIL monitoring and cooling on a high-voltage battery system. Incorrect wiring, flashing, or enabling `PROD` error paths on incomplete hardware can damage equipment or create unsafe conditions. Only flash and run on a correctly assembled, reviewed BMS Master PCB with proper HV safety procedures.

> **NOT HARDWARE-VALIDATED**  
> No functional on-board tests have been conducted. Firmware flash and validation are blocked until the PCB is soldered. Features listed as “done” mean **implemented in source**, not **proven on hardware**.

> **`PROD` builds**  
> When `#define PROD` is enabled in `BMS_Types.h`, temperature and HVIL/safe-state faults are reported through the error handler and can escalate to safe-state behaviour. Keep `PROD` disabled until sensing and CAN paths are verified.

---

## Tech stack

| Layer | Details |
|-------|---------|
| MCU | STM32F105R8Tx, Cortex-M3, LQFP64 |
| HAL / Cube | STM32Cube FW_F1 **V1.8.6**, STM32CubeIDE, CubeMX (`.ioc`) |
| Language / build | C, GCC toolchain (STM32CubeIDE) |
| OS | Bare-metal (no RTOS / FreeRTOS) |
| App layer | `BMS_Driver/` — BMS, ADC, CAN, PWM |
| Driver layer | `EKO_Drivers/` — ADC, CAN, PWM, error handler, LED |
| Buses | CAN1 + CAN2 @ **500 kbit/s**, USART1/USART2 @ **115200** |
| Debug / flash | SWD (ST-LINK V2) |

---

## Features (implemented in code)

| Area | Status |
|------|--------|
| ADC — pack voltage, current, on-board NTC temperature | Done (code) |
| CAN1 — scheduled TX (voltage/current/temp, thermistor groups) | Done (code) |
| CAN2 — RX of cell temperatures / safe-state, ISR + critical section | Done (code) |
| PWM — relay drive (startup → operational duty/frequency) | Done (code) |
| FAN control — hysteresis on max temperature (`PRE`/`POST` cooling) | Done (code) |
| HVIL / safe-state consistency check (`PROD` → EH report) | Done (code) |
| BMS modes — normal / error, peripheral start/stop, status LEDs | Done (code) |
| Error handler integration | Done (code) |

---

## To do

- **Low-power / deep sleep** — STOP/STANDBY (or equivalent), wake-up (e.g. `PA0` WKUP), and peripheral shutdown/restore.
- **RS485 ↔ BMS JK by Dikong** — protocol + app layer on USART2 with `RS_DIR` / `RE_DIR` (CubeMX pins ready; firmware not started).

---

## Notes

- Current development branch focus: `2_0_1/Build/FAN_Control` (FAN control + documentation pass).
- ADC uses DMA circular mode (3 channels: temp / Hall current / voltage).
- CAN transceiver standby is driven by `nCAN1_Stby` / `nCAN2_Stby` (active in operational mode).
- USART1 is reserved for logging / debug UART; USART2 + RS485 direction pins are reserved for the JK BMS link.
- Radio-related GPIOs (`TX_EN`, `TRX_CE`, `PWR_UP`, `DR`, `AM`, `CD`) are defined in CubeMX/`main.h` but **radio firmware is not implemented** in this release.
- Scaling gains/offsets for CAN TX live in `BMS_CAN_driver.h` — recalibrate after first hardware bring-up.
- Cooling thresholds: `PRE_COOLING_TEMP` = 50 °C, `POST_COOLING_TEMP` = 40 °C (`BMS_Types.h`).

---

## Pinout (from CubeMX / `main.h`)

### Analog (ADC1)

| Pin | Signal | Function |
|-----|--------|----------|
| PC0 | `TEMP` / ADC_IN10 | On-board NTC temperature |
| PC1 | `HALL_OUT` / ADC_IN11 | Hall current sense |
| PC2 | `VOLTAGE` / ADC_IN12 | Pack voltage sense |

### CAN

| Pin | Signal | Function |
|-----|--------|----------|
| PA11 | CAN1_RX | Vehicle / host CAN RX |
| PA12 | CAN1_TX | Vehicle / host CAN TX |
| PB12 | CAN2_RX | Cell / slave CAN RX |
| PB13 | CAN2_TX | Cell / slave CAN TX |
| PC11 | `nCAN1_Stby` | CAN1 transceiver standby |
| PC10 | `nCAN2_Stby` | CAN2 transceiver standby |

### UART / RS485 (planned)

| Pin | Signal | Function |
|-----|--------|----------|
| PA9 | USART1_TX | Debug / logger TX |
| PA10 | USART1_RX | Debug / logger RX |
| PA2 | USART2_TX | RS485 data TX (JK BMS — TBD) |
| PA3 | USART2_RX | RS485 data RX (JK BMS — TBD) |
| PC4 | `RS_DIR` | RS485 driver direction |
| PC5 | `RE_DIR` | RS485 receiver enable |

### Actuators / safety / status

| Pin | Signal | Function |
|-----|--------|----------|
| PB0 | `RELAY_CTRL` / TIM3_CH3 | Relay PWM output |
| PB1 | `FAN_CONTROL` | Cooling FAN GPIO |
| PA7 | `HVIL` | HV interlock sense (input) |
| PB8 | `RED_LD` | Error status LED |
| PB9 | `GREEN_LD` | Normal status LED |
| PA0 | SYS_WKUP | Wake-up (for future low-power) |

### Debug / clock

| Pin | Signal | Function |
|-----|--------|----------|
| PA13 | SWDIO | ST-LINK SWD |
| PA14 | SWCLK | ST-LINK SWD |
| PD0 / PD1 | OSC_IN / OSC_OUT | HSE crystal |

### Reserved (radio — not used in FW yet)

| Pin | Signal |
|-----|--------|
| PB14 | `TX_EN` |
| PB15 | `TRX_CE` |
| PC6 | `PWR_UP` |
| PC8 | `DR` |
| PC9 | `AM` |
| PA8 | `CD` |

---

## How to run

1. Open the project in **STM32CubeIDE** (`BMS-Master.ioc` / workspace).
2. Build for **STM32F105R8Tx**.
3. Flash via **ST-LINK V2** only after the PCB is assembled, inspected, and safely powered.
4. Verify CAN termination, ADC sense ranges, relay/FAN wiring, and HVIL before enabling `PROD`.

---

## Authors

- **Bartosz Rychlicki** — Firmware  
- **Szymon Frączek & Wiktor Klaszczyk** — PCB
