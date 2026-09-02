# ADC Module

## Purpose

The ADC module handles the system measurements used by the BMS Master:

- pack voltage,
- current,
- onboard temperature,
- conversion scaling and packaging into CAN frames.

## Main responsibilities

- initialize ADC peripheral and DMA settings,
- read the configured channels,
- convert raw ADC values into engineering units,
- prepare the data for CAN1 transmission,
- support scaling and calibration constants.

## Signals handled

- `TEMP` / ADC_IN10: board NTC temperature
- `HALL_OUT` / ADC_IN11: current sense
- `VOLTAGE` / ADC_IN12: pack voltage sense

## Key design points

- ADC conversion is performed using DMA circular mode.
- The values are stored in `bms->bmsADC.ADC_voltTempCurr[]` before being packed into CAN frames.
- Scaling happens through the formulas defined in the CAN driver header, and final output is sent as a packed CAN1 payload.

## Related files

- `BMS_Driver/ADC/Inc/`
- `BMS_Driver/ADC/Src/`
- `BMS_Driver/CAN/Inc/BMS_CAN_driver.h`
- `BMS_Driver/CAN/Src/BMS_CAN_driver.c`

## Typical flow

1. ADC reads the configured channels.
2. Values are converted and stored.
3. CAN frame generator packs the data.
4. The scheduled CAN1 transmission sends the result to the vehicle bus or host system.
