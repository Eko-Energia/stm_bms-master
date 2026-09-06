# PWM Module

## Purpose

The PWM module is responsible for the BMS control output used by the relay and related power-stage dynamics.

## Responsibilities

- initialize the timer-based PWM output,
- configure relay duty cycle and frequency,
- control start-up and normal-operation behaviour,
- integrate with BMS operating states and fault handling,
- support safe shutdown or error-state operation.

## Related system elements

- Relay control output: `RELAY_CTRL` / TIM3_CH3
- FAN control output: dedicated GPIO, not strictly a PWM channel in the current project layout
- Safe-state transitions and BMS mode changes are coordinated through the BMS layer

## Important notes

The PWM path is tightly coupled to the BMS state machine. During abnormal conditions, the firmware may stop or reset the PWM output to prevent unintended power activation.

## Related files

- `BMS_Driver/PWM/Inc/`
- `BMS_Driver/PWM/Src/`
- `BMS_Driver/BMS/Inc/BMS.h`
- `BMS_Driver/BMS/Src/BMS.c`

## Typical flow

1. BMS enters a mode.
2. PWM driver is started or reconfigured.
3. Relay output follows the required duty cycle.
4. On error, output is disabled or reduced to a safe fallback.
