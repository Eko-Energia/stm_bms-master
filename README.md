# BMS Master — Firmware

The **BMS Master** is the central control unit for a Battery Management System. It is designed to monitor battery health (voltage, current, temperature), manage high-level communication via **CAN buses** and **RS485**, and provide long-range telemetry via an **nRF905 radio transceiver**.

## Key Features

* **Dual CAN Bus Communication:** Integrated support for both the main vehicle CAN (CAN1) and dedicated battery pack communication (CAN2).
* **Radio Telemetry:** Wireless communication using the **nRF905** transceiver via SPI interface.
* **Precision Sensing:** Real-time monitoring of battery voltage, current (Hall sensor), and temperature via ADC.
* **RS485 Interface:** Dedicated communication link for **JK BMS (Dikong)** integration.
* **Abstraction Layer:** Built using a custom peripheral driver layer to standardize the workflow and simplify debugging across the STM32 platform.

---

## Hardware Pinout Configuration

The following table describes the pin mapping for the STM32 MCU as configured on the BMS Master PCB.

| PINOUT | STM PIN | DESCRIPTION |
| :--- | :--- | :--- |
| **PC14_OSC32_IN** | PC14 | Oscilloscope Input 16MHz 1 |
| **PD0_OSC_IN** | PD0 | Oscilloscope Input 16MHz 2 |
| **TEMP** | PC0 | Temperature Measurement (ADC) |
| **HALL_OUT** | PC1 | Current Measurement (Hall Sensor via ADC) |
| **VOLTAGE** | PC2 | Battery Voltage Measurement (ADC) |
| **USART2_TX/RC** | PA2 / PA3 | Debugging Interface |
| **SPI_NSS** | PA4 | nRF905 SPI Enable (Active Low) |
| **SPI_SCK** | PA5 | nRF905 SPI Clock |
| **SPI_MISO** | PA6 | nRF905 SPI Output |
| **SPI_MOSI** | PA7 | nRF905 SPI Input |
| **D1 / D0** | PB0 / PB1 | Radio Amplifier Control |
| **BOOT1** | PB2 | Secondary Boot Pin |
| **CAN2_RX / TX** | PB12 / PB13 | Battery Pack CAN Bus |
| **TX_EN** | PB14 | nRF905 Mode (1=TX, 0=RX) |
| **TRX_CE** | PB15 | nRF905 Chip Enable (RX/TX) |
| **PWR_UP** | PC6 | nRF905 Power Up |
| **UPCLK** | PC7 | nRF905 Output Clock |
| **DR** | PC8 | nRF905 Data Ready |
| **AM** | PC9 | nRF905 Address Match |
| **CD** | PA8 | nRF905 Carrier Detect |
| **USART1_TX/RX** | PA9 / PA10 | RS485 Communication (BMS JK / Dikong) |
| **CAN1_RX / TX** | PA11 / PA12 | Main Vehicle CAN (500 kbit/s) |
| **SWDIO / SWCLK** | PA13 / PA14 | Programming & Debugging (ST-Link) |
| **LED_RED** | PB8 | Onboard Status LED (Red) |
| **LED_GREEN** | PB9 | Onboard Status LED (Green) |
| **RE_DIR / RS_DIR**| PC5 / PC4 | RS485 Transceiver Direction Control (Receive/Send) |

---

## Project Status

### ✅ Completed (Module logic implemented)
* **ADC Sensing:** Reliable reading of Voltage, Current, and Temperature values.
* **Radio Communication:** nRF905 transceiver logic and SPI driver fully operational.
* **CAN Communication:** Both CAN1 (Vehicle) and CAN2 (Battery) interfaces are functional.

### 🟡 Under Construction
* **RS485 Communication:** Implementation of the protocol for JK BMS (Dikong) integration.
* **Error Management:** Developing comprehensive error reporting and handling via `Error_Handler`.

### 🧪 Testing & Validation
> [!IMPORTANT]
> While the core logic for **CAN**, **Radio**, and **ADC** modules is completed, they are currently in the **validation phase**. Rigorous hardware-in-the-loop testing is required to ensure stability and accuracy under real-world conditions before final deployment.

---

## How to Run

1.  **Hardware Connection:** Ensure the BMS Master PCB is correctly powered and all sensors (Hall, Voltage divider, NTC) are connected to the designated pins.
2.  **Development Environment:** Open the project in your preferred STM32 IDE (e.g., STM32CubeIDE).
3.  **Flashing:** Connect an **ST-LINK V2** to the SWD pins (PA13/PA14) and upload the firmware.

---

## Authors

* **Bartosz Rychlicki** – Firmware
* **Szymon Frączek & Wiktor Klaszczyk** – PCB Design
