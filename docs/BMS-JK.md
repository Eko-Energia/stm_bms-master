# BMS JK RS485 / CAN bridge

This note defines the project-side trigger commands and the CAN frames used to expose JK BMS telemetry to the rest of the system through the RS485 link and the onboard CAN bus.

## 1) UART / RS485 trigger commands

The JK link is routed through USART2 and the RS485 driver (`RS_DIR` / `RE_DIR`). The BM_JK app should trigger a read operation by sending one of the following request commands on the UART side.

These are intentionally kept simple and deterministic so they can be mapped directly to a request/response flow in the Python app.

| Command ID | Name                   | Purpose                                                    |
| ---------- | ---------------------- | ---------------------------------------------------------- |
| `0xB1`     | `JK_REQ_PACK_INFO`     | Request pack summary: voltage, current, SOC, power, status |
| `0xB2`     | `JK_REQ_CELL_VOLTAGES` | Request all cell voltages                                  |
| `0xB3`     | `JK_REQ_TEMPERATURES`  | Request cell / MOS temp information                        |
| `0xB4`     | `JK_REQ_STATUS`        | Request alarms, protections and mode                       |
| `0xBF`     | `JK_REQ_ALL`           | Request a full snapshot                                    |

### Suggested UART packet format

Use a compact frame like this:

```text
[SOF] [CMD] [LEN] [PAYLOAD...] [CRC]
```

Example request for a full dump:

```text
7E B1 01 FF 00
```

Example request for cell voltages only:

```text
7E B2 01 FF 00
```

The exact CRC implementation may stay app-defined (`XOR`, `CRC16-CCITT`, or `sum8`), but the command IDs above should remain stable for the firmware / app integration.

### RS485 direction handling

For the STM32 side:

- `RS_DIR = 1` => transmit request to JK BMS
- `RS_DIR = 0` => receive response
- `RE_DIR` should follow the same direction policy as the transceiver enable

The BM_JK Python app can be configured to send one command, wait for the response, and then forward the decoded values into the CAN payload format below.

---

## 2) Proposed CAN frames for BMS master export

The BMS master should expose JK telemetry on CAN1 using a dedicated block of IDs that does not collide with the existing thermistor / ADC packets.

### Reserved CAN IDs

For this 21-cell pack, the cell voltage and temperature streams are grouped in 3 blocks of 7 values each.

| CAN ID  | Name                     | Payload summary                               |
| ------- | ------------------------ | --------------------------------------------- |
| `0x140` | `BMS_JK_PACK_INFO`       | pack voltage, current, SOC, SOH, status, mode |
| `0x141` | `BMS_JK_CELL_VOLT_1_7`   | cells 1..7 voltages                           |
| `0x142` | `BMS_JK_CELL_VOLT_8_14`  | cells 8..14 voltages                          |
| `0x143` | `BMS_JK_CELL_VOLT_15_21` | cells 15..21 voltages                         |
| `0x144` | `BMS_JK_TEMP_1_7`        | temperatures 1..7                             |
| `0x145` | `BMS_JK_TEMP_8_14`       | temperatures 8..14                            |
| `0x146` | `BMS_JK_TEMP_15_21`      | temperatures 15..21                           |
| `0x147` | `BMS_JK_ALARM_STATUS`    | protection flags, error state, balance state  |
| `0x148` | `BMS_JK_CYCLE_STATS`     | cycle count / capacity / resistance / time    |

> Note: for standard CAN (8-byte payloads), a raw 16-bit cell voltage value does not fit 7 values in one frame. The 7-value groups are therefore the logical DBC grouping, while the frame payload is packed with the recommended standard-CAN bit layout (e.g. `4+3` or `3+2+2` 16-bit values per message depending on the exact packing). This is still easy to model in DBC and keeps the update rate at 1000 ms.

### DBC-ready periodic CAN export (1000 ms)

The JK telemetry should be exported on CAN1 as periodic frames every 1000 ms. In DBC terms, use:

- `GenMsgCycleTime = 1000` for each message
- byte order: `Intel` / little-endian
- signal type: `unsigned` for flags and counters, `signed` for current/temperature
- factor/offset convention kept consistent with the JK PDF values:
  - `pack_voltage_V = raw * 0.01 + 0.0`
  - `pack_current_A = raw * 0.01 + 0.0` (signed)
  - `cell_voltage_V = raw * 0.001 + 0.0`
  - `temp_C = raw * 0.1 + 0.0`
  - `soc_pct = raw * 1 + 0`
  - `capacity_mAh = raw * 1 + 0`
  - status flags: raw bitfield, no scaling

This keeps the frame layout simple and matches the code already using `mV`, `mA`, `mAh`, and `1%` SOC values.

#### Summary table

| CAN ID  | DBC message name     | DLC | Period  | Content                                          |
| ------- | -------------------- | --- | ------- | ------------------------------------------------ |
| `0x140` | `JK_PACK_INFO`       | 8   | 1000 ms | pack voltage, current, SOC, SOH, status, mode    |
| `0x141` | `JK_CELL_VOLT_1_7`   | 8   | 1000 ms | logical block: cells 1..7                        |
| `0x142` | `JK_CELL_VOLT_8_14`  | 8   | 1000 ms | logical block: cells 8..14                       |
| `0x143` | `JK_CELL_VOLT_15_21` | 8   | 1000 ms | logical block: cells 15..21                      |
| `0x144` | `JK_TEMP_1_7`        | 8   | 1000 ms | logical block: temperatures 1..7                 |
| `0x145` | `JK_TEMP_8_14`       | 8   | 1000 ms | logical block: temperatures 8..14                |
| `0x146` | `JK_TEMP_15_21`      | 8   | 1000 ms | logical block: temperatures 15..21               |
| `0x147` | `JK_ALARM_STATUS`    | 8   | 1000 ms | protection flags, balance, faults, mode          |
| `0x148` | `JK_CYCLE_STATS`     | 8   | 1000 ms | cycle count, remaining/full capacity, resistance |

For the temperature blocks, 7 values x 8-bit `0.1 °C` fields fit easily in a standard CAN payload. For cell voltages, use the same 7-value logical grouping but pack them as 16-bit entries using the standard-CAN bit-packing scheme (`4+3` per frame or equivalent), keeping the same DBC signal names and 1000 ms rate.

#### `0x140` — `JK_PACK_INFO`

| Signal        | Start bit | Length | Byte order | Value type | Factor | Offset | Unit     | Notes                                               |
| ------------- | --------- | ------ | ---------- | ---------- | ------ | ------ | -------- | --------------------------------------------------- |
| `PackVoltage` | 0         | 16     | Intel      | Unsigned   | 0.01   | 0      | V        | pack voltage                                        |
| `PackCurrent` | 16        | 16     | Intel      | Signed     | 0.01   | 0      | A        | positive = charge/discharge sign as reported by BMS |
| `SOC`         | 32        | 8      | Intel      | Unsigned   | 1      | 0      | %        | state of charge                                     |
| `SOH`         | 40        | 8      | Intel      | Unsigned   | 1      | 0      | %        | state of health                                     |
| `StatusFlags` | 48        | 8      | Intel      | Unsigned   | 1      | 0      | bitfield | main status bits                                    |
| `ModeFlags`   | 56        | 8      | Intel      | Unsigned   | 1      | 0      | bitfield | mode / protection bits                              |

Example bit assignments:

- `StatusFlags`:
  - bit0: charging
  - bit1: discharging
  - bit2: balance active
  - bit3: protection active
  - bit4: temp high alarm
  - bit5: temp low alarm
  - bit6: communication error
  - bit7: general fault
- `ModeFlags`:
  - bit0: sleep/standby
  - bit1: normal
  - bit2: balancing
  - bit3: charge limit active
  - bit4: discharge limit active
  - bit5..7: reserved

#### `0x141` — `JK_CELL_VOLT_1_7`

Logical block for cells 1..7.

| Signal    | Start bit | Length | Byte order | Value type | Factor | Offset | Unit |
| --------- | --------- | ------ | ---------- | ---------- | ------ | ------ | ---- |
| `Cell1_V` | 0         | 16     | Intel      | Unsigned   | 0.001  | 0      | V    |
| `Cell2_V` | 16        | 16     | Intel      | Unsigned   | 0.001  | 0      | V    |
| `Cell3_V` | 32        | 16     | Intel      | Unsigned   | 0.001  | 0      | V    |
| `Cell4_V` | 48        | 16     | Intel      | Unsigned   | 0.001  | 0      | V    |

The same logical block is then continued for the remaining 3 cell voltages in the same message payload using the standard CAN bit-packing convention for a 7-value block.

#### `0x142` — `JK_CELL_VOLT_8_14`

Logical block for cells 8..14.

| Signal     | Start bit | Length | Byte order | Value type | Factor | Offset | Unit |
| ---------- | --------- | ------ | ---------- | ---------- | ------ | ------ | ---- |
| `Cell8_V`  | 0         | 16     | Intel      | Unsigned   | 0.001  | 0      | V    |
| `Cell9_V`  | 16        | 16     | Intel      | Unsigned   | 0.001  | 0      | V    |
| `Cell10_V` | 32        | 16     | Intel      | Unsigned   | 0.001  | 0      | V    |
| `Cell11_V` | 48        | 16     | Intel      | Unsigned   | 0.001  | 0      | V    |

#### `0x143` — `JK_CELL_VOLT_15_21`

Logical block for cells 15..21.

| Signal     | Start bit | Length | Byte order | Value type | Factor | Offset | Unit |
| ---------- | --------- | ------ | ---------- | ---------- | ------ | ------ | ---- |
| `Cell15_V` | 0         | 16     | Intel      | Unsigned   | 0.001  | 0      | V    |
| `Cell16_V` | 16        | 16     | Intel      | Unsigned   | 0.001  | 0      | V    |
| `Cell17_V` | 32        | 16     | Intel      | Unsigned   | 0.001  | 0      | V    |
| `Cell18_V` | 48        | 16     | Intel      | Unsigned   | 0.001  | 0      | V    |

The final three cell voltages (`Cell19_V`..`Cell21_V`) are placed in the same 7-value block using the same bit-packing rules for the remaining payload space.

#### `0x144` — `JK_TEMP_1_7`

| Signal  | Start bit | Length | Byte order | Value type | Factor | Offset | Unit |
| ------- | --------- | ------ | ---------- | ---------- | ------ | ------ | ---- |
| `Temp1` | 0         | 8      | Intel      | Signed     | 0.1    | 0      | °C   |
| `Temp2` | 8         | 8      | Intel      | Signed     | 0.1    | 0      | °C   |
| `Temp3` | 16        | 8      | Intel      | Signed     | 0.1    | 0      | °C   |
| `Temp4` | 24        | 8      | Intel      | Signed     | 0.1    | 0      | °C   |
| `Temp5` | 32        | 8      | Intel      | Signed     | 0.1    | 0      | °C   |
| `Temp6` | 40        | 8      | Intel      | Signed     | 0.1    | 0      | °C   |
| `Temp7` | 48        | 8      | Intel      | Signed     | 0.1    | 0      | °C   |

#### `0x145` — `JK_TEMP_8_14`

| Signal   | Start bit | Length | Byte order | Value type | Factor | Offset | Unit |
| -------- | --------- | ------ | ---------- | ---------- | ------ | ------ | ---- |
| `Temp8`  | 0         | 8      | Intel      | Signed     | 0.1    | 0      | °C   |
| `Temp9`  | 8         | 8      | Intel      | Signed     | 0.1    | 0      | °C   |
| `Temp10` | 16        | 8      | Intel      | Signed     | 0.1    | 0      | °C   |
| `Temp11` | 24        | 8      | Intel      | Signed     | 0.1    | 0      | °C   |
| `Temp12` | 32        | 8      | Intel      | Signed     | 0.1    | 0      | °C   |
| `Temp13` | 40        | 8      | Intel      | Signed     | 0.1    | 0      | °C   |
| `Temp14` | 48        | 8      | Intel      | Signed     | 0.1    | 0      | °C   |

#### `0x146` — `JK_TEMP_15_21`

| Signal   | Start bit | Length | Byte order | Value type | Factor | Offset | Unit |
| -------- | --------- | ------ | ---------- | ---------- | ------ | ------ | ---- |
| `Temp15` | 0         | 8      | Intel      | Signed     | 0.1    | 0      | °C   |
| `Temp16` | 8         | 8      | Intel      | Signed     | 0.1    | 0      | °C   |
| `Temp17` | 16        | 8      | Intel      | Signed     | 0.1    | 0      | °C   |
| `Temp18` | 24        | 8      | Intel      | Signed     | 0.1    | 0      | °C   |
| `Temp19` | 32        | 8      | Intel      | Signed     | 0.1    | 0      | °C   |
| `Temp20` | 40        | 8      | Intel      | Signed     | 0.1    | 0      | °C   |
| `Temp21` | 48        | 8      | Intel      | Signed     | 0.1    | 0      | °C   |

`Temp*` blocks are the cleanest DBC format since 7 x 8-bit values fit naturally in one standard CAN payload. This also gives a straightforward mapping to a Jikong temperature block.

#### `0x147` — `JK_ALARM_STATUS`

| Signal               | Start bit | Length | Byte order | Value type | Factor | Offset | Unit |
| -------------------- | --------- | ------ | ---------- | ---------- | ------ | ------ | ---- |
| `OVP_Fault`          | 0         | 8      | Intel      | Unsigned   | 1      | 0      | bit  |
| `UVP_Fault`          | 8         | 8      | Intel      | Unsigned   | 1      | 0      | bit  |
| `OCP_Fault`          | 16        | 8      | Intel      | Unsigned   | 1      | 0      | bit  |
| `ShortCircuit_Fault` | 24        | 8      | Intel      | Unsigned   | 1      | 0      | bit  |
| `OverTemp_Fault`     | 32        | 8      | Intel      | Unsigned   | 1      | 0      | bit  |
| `UnderTemp_Fault`    | 40        | 8      | Intel      | Unsigned   | 1      | 0      | bit  |
| `BalanceActive`      | 48        | 8      | Intel      | Unsigned   | 1      | 0      | bit  |
| `ProtectionLatch`    | 56        | 8      | Intel      | Unsigned   | 1      | 0      | bit  |

This is the compact status field used to map the JK alarm / protection register into a DBC bitfield.

#### `0x148` — `JK_CYCLE_STATS`

| Signal               | Start bit | Length | Byte order | Value type | Factor | Offset | Unit   |
| -------------------- | --------- | ------ | ---------- | ---------- | ------ | ------ | ------ |
| `CycleCount`         | 0         | 16     | Intel      | Unsigned   | 1      | 0      | cycles |
| `RemainingCapacity`  | 16        | 32     | Intel      | Unsigned   | 1      | 0      | mAh    |
| `FullCapacity`       | 48        | 32     | Intel      | Unsigned   | 1      | 0      | mAh    |
| `ResistanceEstimate` | 80        | 8      | Intel      | Unsigned   | 1      | 0      | mOhm   |
| `Reserved`           | 88        | 8      | Intel      | Unsigned   | 1      | 0      | -      |

For a strict 8-byte DLC, the resistance / reserved byte can be kept in a second message if a larger capacity or resistance range is required. For the current Jikong protocol, the main values are already sufficient for a DBC import and a 1000 ms telemetry cycle.

- bit0: cell temp high
- bit1: cell temp low
- bit2: MOS temp high
- bit3: ambient temp high
- bit4: sensor open/short
- bit5..7: reserved

#### `0x144` — `JK_ALARM_STATUS`

| Signal               | Start bit | Length | Byte order | Value type | Factor | Offset | Unit |
| -------------------- | --------- | ------ | ---------- | ---------- | ------ | ------ | ---- |
| `OVP_Fault`          | 0         | 8      | Intel      | Unsigned   | 1      | 0      | bit  |
| `UVP_Fault`          | 8         | 8      | Intel      | Unsigned   | 1      | 0      | bit  |
| `OCP_Fault`          | 16        | 8      | Intel      | Unsigned   | 1      | 0      | bit  |
| `ShortCircuit_Fault` | 24        | 8      | Intel      | Unsigned   | 1      | 0      | bit  |
| `OverTemp_Fault`     | 32        | 8      | Intel      | Unsigned   | 1      | 0      | bit  |
| `UnderTemp_Fault`    | 40        | 8      | Intel      | Unsigned   | 1      | 0      | bit  |
| `BalanceActive`      | 48        | 8      | Intel      | Unsigned   | 1      | 0      | bit  |
| `ProtectionLatch`    | 56        | 8      | Intel      | Unsigned   | 1      | 0      | bit  |

This is the compact status field used to map the JK alarm / protection register into a DBC bitfield.

#### `0x145` — `JK_CYCLE_STATS`

| Signal               | Start bit | Length | Byte order | Value type | Factor | Offset | Unit   |
| -------------------- | --------- | ------ | ---------- | ---------- | ------ | ------ | ------ |
| `CycleCount`         | 0         | 16     | Intel      | Unsigned   | 1      | 0      | cycles |
| `RemainingCapacity`  | 16        | 32     | Intel      | Unsigned   | 1      | 0      | mAh    |
| `FullCapacity`       | 48        | 32     | Intel      | Unsigned   | 1      | 0      | mAh    |
| `ResistanceEstimate` | 80        | 8      | Intel      | Unsigned   | 1      | 0      | mOhm   |
| `Reserved`           | 88        | 8      | Intel      | Unsigned   | 1      | 0      | -      |

For a strict 8-byte DLC, the resistance / reserved byte can be kept in a second message if a larger capacity or resistance range is required. For the current Jikong protocol, the main values are already sufficient for a DBC import and a 1000 ms telemetry cycle.

---

## 3) Recommended BM_JK app flow

1. Connect to the JK BMS over the RS485 UART port.
2. Send `JK_REQ_ALL` (`0xBF`) to request the full snapshot.
3. Parse the response and decode the values into:
   - pack voltage/current
   - cell voltages
   - temperature set
   - alarm bits
   - state / mode / SOC
4. Publish the decoded values to CAN using the message set above.

A practical sequence is:

```text
BM_JK app -> UART: 7E B0 BF 00 00     # request full snapshot
BM_JK app <- UART: JK response frame
BM_JK app -> CAN: 0x140, 0x141, 0x142, 0x143, 0x144, 0x145
```

The actual app can keep the transmit loop around 1000 ms and re-send the same message set when the JK response is refreshed.

---

## 4) Implementation note

This export is intentionally kept separate from the existing ADC / thermistor CAN traffic, so the JK bridge remains easy to decode in a DBC and does not interfere with the pack-sensing frames. The sender rate stays at `~1000 ms` to match the expected JK polling cadence and to keep bus load low while still providing a useful telemetry update.

---

## 3) Recommended BM_JK app flow

1. Connect to the JK BMS over the RS485 UART port.
2. Send `JK_REQ_ALL` (`0xBF`) to request the full snapshot.
3. Parse the response and decode the values into:
   - pack voltage/current
   - cell voltages
   - temperature set
   - alarm bits
   - state / mode / SOC
4. Publish the decoded values to CAN using the new frame IDs above.

A practical sequence is:

```text
BM_JK app -> UART: 7E B0 BF 00 00     # request full snapshot
BM_JK app <- UART: JK response frame
BM_JK app -> CAN: 0x140, 0x141, 0x142, 0x143, 0x144, 0x145
```

---

## 4) Implementation note

This frame set is meant to be used as the standard telemetry export from the JK bridge into the project CAN network. It complements the existing BMS ADC and thermistor CAN frames, while keeping a separate JK-oriented block for diagnostics and monitoring.

The final encoding (`mV`, `0.1 A`, `1% SOC`) should be chosen once the BM_JK app is validated against a real JK BMS response.
