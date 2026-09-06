# BMS Core Module

## Role

The BMS core module is the main application layer that connects the peripheral drivers and state machine.

## Responsibilities

- initialize ADC, CAN, PWM and JK bridge objects,
- manage operating mode transitions,
- set up RS485 direction control,
- handle safe-state and fault states,
- monitor temperature and fan behavior,
- coordinate the runtime debug view through the main BMS structure.

## Main runtime object

The central BMS object is defined in `BMS_Driver/BMS/Inc/BMS_Types.h` and includes:

- `bmsADC` for ADC data
- `bmsCAN` for CAN1/CAN2 state
- `bmsJK` for JK telemetry snapshot and RS485 bridge state
- `bpwm` for PWM/relay behavior
- `beh` for error handler integration

This makes the JK battery telemetry directly inspectable from the top-level BMS object during debugging.

## Initialization flow

1. `BMS_Init()` assigns peripheral handles.
2. `BMS_JK_Init()` initializes the JK bridge snapshot context.
3. PWM is started before the rest of the app to preserve safe relay behavior.
4. CAN and ADC are initialized.
5. Error handler and runtime defaults are set.

## Debugging note

When debugging or tracing the BMS runtime, the most useful live objects are:

- `bms->bmsADC.ADC_voltTempCurr[]`
- `bms->bmsCAN.CAN2_temperatureCells[][]`
- `bms->bmsJK.snapshot`
- `bms->maxTemperature`
- `bms->safeStateStatus`

These provide a focused view of the core pack values and the JK bridge data at the same time.
