# BMS Master

The BMS Master project provides a robust implementation of essential Battery Management System functionalities.

### Key Features

- **Dual CAN Communication:** Full support for CAN1 and CAN2 buses.
- **ADC Data Acquisition:** Precise measurement of main battery parameters, including voltage, current, and temperature with precisly selected sampling time.
- **Standardized Hardware Abstraction:** Built on a custom peripheral driver layer designed to simplify debugging and unify the workflow across different STM32 microcontrollers.

### Hardware Pinout

| Peripheral     | Signal             | STM32 Pin       | Description                            |
| :------------- | :----------------- | :-------------- | :------------------------------------- |
| **CAN1**       | RX / TX            | PA11 / PA12     | Main Vehicle Bus                       |
| **CAN2**       | RX / TX            | PB12 / PB13     | Battery Pack Bus                       |
| **ADC1**       | IN10 / IN11 / IN12 | PC0 / PC1 / PC2 | Analog Inputs (Temp, Current, Voltage) |
| **UART1**      | TX / RX            | PA9 / PA10      | JK BMS Interface                       |
| **UART2**      | TX / RX            | PA2 / PA3       | Debug Console (CLI)                    |
| **RS485 CTL**  | RE / DE            | PC5 / PC4       | RS485 Direction Control                |
| **System**     | OSC_IN / OSC_OUT   | PH0 / PH1       | External High Speed Oscillator (HSE)   |
| **Status LED** | RED / GREEN        | PB8 / PB9       | Error & Status Indicators              |
| **Debug**      | SWDIO / SWCLK      | PA13 / PA14     | ST-Link Interface                      |

### Peripheral Features & Configuration

#### 📡 Communication

- **CAN1 & CAN2:** High-speed CAN configuration (500 kbit/s). CAN2 operates with RX FIFO0 Pending Interrupt enabled for asynchronous data frame processing.
- **UART1 (BMS JK):** 115200 8N1, optimized for RS485 half-duplex communication with external BMS.
- **UART2 (Error Logger):** Standard asynchronous mode for real-time system logging.

#### ⚡ Data Acquisition (ADC)

- **Multi-Channel Scanning:** ADC1 configured for sequential scanning of Temperature, Current, and Voltage sensors.
- **DMA Integration:** Utilizes Circular DMA Buffer to offload the CPU, ensuring continuous background data updates without polling overhead.

#### ⏱️ System & Clock

- **HSE (External Oscillator):** System clock driven by an external crystal oscillator for high frequency stability, essential for reliable CAN bus timing.

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

### 🚀 Future Roadmap

- **RTOS Integration:** Migration of the current bare-metal architecture to a Real-Time Operating System (Azure RTOS or FreeRTOS) to improve multi-threading capabilities, especially for simultaneous CAN communication and radio handling.
- **Low-Power Optimization:** Implementation of advanced power management, including STM32 "Stop" and "Standby" modes, with wake-up events triggered by CAN traffic or nRF905 signals to minimize stationary energy consumption.
- **SIL Testing Environment:** Transition from on-target TDD (CUnit) to Software-in-the-Loop (SIL) testing using Google Test (GTest) for faster logic verification and CI/CD readiness.
- **RS485 Full Stack:** Finalizing the communication layer for the JK BMS to enable full battery pack diagnostics and balancing control.

### Development Team

- **Bartosz Rychlicki** – Firmware Engineering
- **Szymon Frączek & Wiktor Klaszczyk** – PCB Design & Hardware
