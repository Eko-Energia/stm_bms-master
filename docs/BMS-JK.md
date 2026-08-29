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

| CAN ID  | Name                     | Payload summary                              |
| ------- | ------------------------ | -------------------------------------------- |
| `0x140` | `BMS_JK_PACK_INFO`       | pack voltage, current, SOC, SOH, status      |
| `0x141` | `BMS_JK_CELL_VOLTS_1_8`  | cells 1..8 voltages                          |
| `0x142` | `BMS_JK_CELL_VOLTS_9_16` | cells 9..16 voltages                         |
| `0x143` | `BMS_JK_TEMP_SUMMARY`    | temp sensor block (cell / MOS / ambient)     |
| `0x144` | `BMS_JK_ALARM_STATUS`    | protection flags, error state, balance state |
| `0x145` | `BMS_JK_CYCLE_STATS`     | cycle count / capacity / resistance / time   |

### Frame details

#### `0x140` — BMS_JK_PACK_INFO

DLC: `8`

```text
byte0: pack voltage low
byte1: pack voltage high
byte2: current low
byte3: current high
byte4: SOC low (0.5% resolution or 1% depending on app format)
byte5: SOH low
byte6: status flags
byte7: reserved / checksum / mode
```

Scale rules:

- pack voltage: `mV` or `0.01 V` depending on the chosen encoding
- current: signed value in `A` or `0.1 A`
- SOC / SOH: 0..100
- status flags: bitfield (charging, discharging, balance, protection, temp alarm)

#### `0x141` — BMS_JK_CELL_VOLTS_1_8

DLC: `8`

```text
byte0..1: cell 1 voltage (mV)
byte2..3: cell 2 voltage (mV)
byte4..5: cell 3 voltage (mV)
byte6..7: cell 4 voltage (mV)
```

The second block is used for the remaining cells:

#### `0x142` — BMS_JK_CELL_VOLTS_9_16

DLC: `8`

```text
byte0..1: cell 5 voltage (mV)
byte2..3: cell 6 voltage (mV)
byte4..5: cell 7 voltage (mV)
byte6..7: cell 8 voltage (mV)
```

If the pack has more than 8 cells, extend the layout with additional frame IDs or use a fixed 16-cell block mapping.

#### `0x143` — BMS_JK_TEMP_SUMMARY

DLC: `8`

```text
byte0: min cell temp
byte1: max cell temp
byte2: avg cell temp
byte3: MOS temp
byte4: ambient temp
byte5: sensor count
byte6: temp alarm flags
byte7: reserved
```

#### `0x144` — BMS_JK_ALARM_STATUS

DLC: `8`

```text
byte0: over-voltage flag
byte1: under-voltage flag
byte2: over-current flag
byte3: short-circuit flag
byte4: over-temp flag
byte5: under-temp flag
byte6: balance active flag
byte7: protection latch / error code
```

#### `0x145` — BMS_JK_CYCLE_STATS

DLC: `8`

```text
byte0..1: cycle count
byte2..3: remaining capacity (mAh)
byte4..5: full capacity (mAh)
byte6: internal resistance estimate
byte7: reserved / mode / state
```

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
