# BMS Master

The BMS Master project provides a robust implementation of essential Battery Management System functionalities.

### Key Features

- **Dual CAN Communication:** Full support for CAN1 and CAN2 buses.
- **Radio Communication:** Wireless data transmission capabilities via SPI-connected transceiver.
- **ADC Data Acquisition:** Precise measurement of main battery parameters, including voltage, current, and temperature.
- **Standardized Hardware Abstraction:** Built on a custom peripheral driver layer designed to simplify debugging and unify the workflow across different STM32 microcontrollers.

### Hardware Configuration & Pinout

| Peripheral | Signal      | STM32 Pin   | Mode / Configuration           | Description                         |
| :--------- | :---------- | :---------- | :----------------------------- | :---------------------------------- |
| **CAN1**   | RX / TX     | PA11 / PA12 | 500 kbit/s, Interrupts enabled | Main vehicle communication          |
| **CAN2**   | RX / TX     | PB12 / PB13 | 500 kbit/s, Interrupts enabled | Internal battery pack communication |
| **ADC1**   | IN10        | PC0         | Single-ended, DMA enabled      | Temperature measurement             |
| **ADC1**   | IN11        | PC1         | Single-ended, DMA enabled      | Current measurement (Hall sensor)   |
| **ADC1**   | IN12        | PC2         | Single-ended, DMA enabled      | Battery voltage sensing             |
| **UART1**  | TX / RX     | PA9 / PA10  | 115200 8N1, Asynchronous       | Communication with JK BMS           |
| **UART2**  | TX / RX     | PA2 / PA3   | 115200 8N1, Asynchronous       | Debugging Console (CLI)             |
| **SPI1**   | Full Duplex | PA4-PA7     | Master, NSS Hardware, Mode 0   | Radio transceiver interface         |
| **GPIO**   | RS485_RE    | PC5         | Output Push-Pull               | RS485 Receive Enable Control        |
| **GPIO**   | RS485_DE    | PC4         | Output Push-Pull               | RS485 Driver Enable Control         |
| **GPIO**   | LED_RED     | PB8         | Output Push-Pull               | Error / Status Indicator            |
| **GPIO**   | LED_GRN     | PB9         | Output Push-Pull               | Power / Heartbeat Indicator         |
| **SYS**    | SWD         | PA13 / PA14 | Serial Wire Debug              | Programming and Debug Interface     |

### Project Status

#### ⏳ Pending

- RS485 communication with JK BMS (by Heltec/JiKong).
- PWM-based MOSFET control and power management.

#### 🏗️ In Progress

- None

#### ✅ Completed

- **ADC Driver Implementation:**
  - Voltage monitoring.
  - Current sensing.
  - Temperature measurement.
- **Communication Stacks:**
  - Integrated CAN1 and CAN2 communication drivers.

### Deployment

To deploy the project on the BMS Master PCB:

1. Ensure all hardware connections are correctly established according to the schematics.
2. Flash the firmware onto the STM32 core using an **ST-LINK V2** debugger.

### Development Team

- **Bartosz Rychlicki** – Firmware Engineering
- **Szymon Frączek & Wiktor Klaszczyk** – PCB Design & Hardware
