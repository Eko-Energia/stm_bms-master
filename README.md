# BMS Master

The BMS Master project provides a robust implementation of essential Battery Management System functionalities for **Perła** Solar Car.

### Key Features

- **Dual CAN Communication:** Full support for CAN1 and CAN2 buses.
- **Radio Communication:** Wireless data transmission capabilities.
- **ADC Data Acquisition:** Precise measurement of main battery parameters, including voltage, current, and temperature.
- **Standardized Hardware Abstraction:** Built on a custom peripheral driver layer designed to simplify debugging and unify the workflow across different STM32 microcontrollers.

### Project Status

#### ⏳ Pending

- RS485 communication with JK BMS.
- PWM-based MOSFET control and power management.

#### 🏗️ In Progress

- None

#### ✅ Completed

- **Signals Measurement:**
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
