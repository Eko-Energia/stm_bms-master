# CAN Module

## Purpose

The CAN module implements the BMS communication layer for the two CAN buses:

- CAN1: scheduled outbound telemetry and host-facing data
- CAN2: incoming cell thermistor and safe-state frames

## Responsibilities

- initialize CAN transceivers and filters,
- register periodic TX frames,
- pack ADC and thermistor data into CAN payloads,
- receive CAN2 thermal frames,
- track scan state and temperature maxima,
- expose diagnostics to the BMS state machine.

## CAN1 TX frames

The scheduled CAN1 messages include:

- pack voltage / current / temp payload,
- group frames for thermistor sets from the CAN2 scan,
- safety and status metadata when implemented.

## CAN2 RX handling

- Frames are received through the FIFO callback path.
- Thermistor IDs are decoded by PCB and sensor index.
- The raw value is stored in the local temperature matrix.
- A smoothing filter can be applied before forwarding to the rest of the system.

## Important constants

Relevant definitions are kept in:

- `BMS_Driver/CAN/Inc/BMS_CAN_driver.h`

Important areas include:

- frame IDs,
- DLC definitions,
- periods,
- thermistor scan parameters,
- scaling constants.

## Related files

- `BMS_Driver/CAN/Inc/BMS_CAN_driver.h`
- `BMS_Driver/CAN/Src/BMS_CAN_driver.c`
- `BMS_Driver/BMS/Inc/BMS_Types.h`

## Safety / design notes

- The CAN2 path must not accept invalid thermistor IDs outside the valid PCB / thermistor map.
- Over-temperature behavior is controlled by the BMS logic and can escalate via the error handler in `PROD` builds.
- The transmit path should remain deterministic and low latency to avoid timing jitter on the main vehicle bus.
