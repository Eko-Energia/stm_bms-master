# BMS Master Project Overview

This repository contains the firmware for the BMS Master board based on the STM32F105R8Tx MCU.

## Purpose

The BMS Master is responsible for:

- measuring pack voltage, current and board temperature,
- monitoring cell thermistor data received from the CAN2 bus,
- publishing BMS telemetry over CAN1,
- controlling the relay and cooling fan,
- exposing JK BMS telemetry through the RS485 bridge,
- reporting faults and operating state transitions.

## System layout

### Core software blocks

- BMS application layer: `BMS_Driver/BMS/`
- ADC acquisition driver: `BMS_Driver/ADC/`
- CAN bus layer: `BMS_Driver/CAN/`
- PWM / relay driver: `BMS_Driver/PWM/`
- JK bridge: `BMS_Driver/BMS-JK/`
- HAL-generated core files: `Core/`
- board support and generic drivers: `Drivers/`, `EKO_Drivers/`

### Communication buses

- CAN1: telemetry and host CAN export
- CAN2: cell thermal / slave node receive path
- USART1: debug logging
- USART2: RS485 JK BMS link

## Operational states

The main controller is designed around a BMS state machine:

- normal mode
- error mode
- safe-state monitoring
- relay and fan regulation

The firmware uses hysteresis for cooling decisions and performs fault checks when `PROD` is enabled.

## Main files

- `README.md` — project summary and hardware notes
- `BMS-JK.md` — RS485 command and CAN export specification
- `PROJECT.md` — architecture and integration overview
- `CONTRIBUTORS.md` — project contributors

## Build notes

- Project target: STM32F105R8Tx
- Toolchain: STM32CubeIDE / GCC-based firmware build
- Bus speed: CAN1/CAN2 at 500 kbit/s
- UART speed: 115200 baud

## Safety note

This is high-voltage battery firmware. Before enabling production fault handling, verify all sensing, relay, cooling, and safety wiring carefully.
