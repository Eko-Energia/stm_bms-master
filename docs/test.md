# BMS Master CAN Frames Definition

## 1. Battery Status & SOC Frame (`BATT_ST`)
* **CAN ID:** `0x02F4` (Decimal: 756)
* **Sender:** `BMSMaster`
* **Cycle Time:** 20 ms

| Signal Name | Start Bit | Bit Length | Byte Order | Scaling (Factor, Offset) | Range | Unit | Description |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| `BattVolt` | 0 | 16 | Intel (1+) | (0.1, 0) | [0 \| 1000] | V | Total battery voltage |
| `BattCurr` | 16 | 16 | Intel (1+) | (0.1, -400) | [-400 \| 1000] | A | Total battery current |
| `SOC` | 32 | 8 | Intel (1+) | (1, 0) | [0 \| 100] | % | State of Charge |
| `DischgTime` | 40 | 16 | Intel (1+) | (1, 0) | [0 \| 65535] | h | Remaining discharge time |

---

## 2. Protection & Alarm Information Frame (`ALM_INFO`)
* **CAN ID:** `0x07F4` (Decimal: 2036)
* **Sender:** `BMSMaster`
* **Trigger:** Event-driven (sent periodically when an alarm occurs)

| Signal Name | Start Bit | Bit Length | Byte Order | Scaling (Factor, Offset) | Range | Unit | Description |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| `CellOverVolt` | 0 | 2 | Intel (1+) | (1, 0) | [0 \| 3] | - | Cell over-voltage alarm level |
| `CellUnderVolt` | 2 | 2 | Intel (1+) | (1, 0) | [0 \| 3] | - | Cell under-voltage alarm level |
| `PackOverVolt` | 4 | 2 | Intel (1+) | (1, 0) | [0 \| 3] | - | Pack over-voltage alarm level |
| `PackUnderVolt` | 6 | 2 | Intel (1+) | (1, 0) | [0 \| 3] | - | Pack under-voltage alarm level |
| `CellDiffHigh` | 8 | 2 | Intel (1+) | (1, 0) | [0 \| 3] | - | Cell voltage difference high alarm |
| `DischargeOC` | 10 | 2 | Intel (1+) | (1, 0) | [0 \| 3] | - | Discharge over-current alarm |
| `ChargeOC` | 12 | 2 | Intel (1+) | (1, 0) | [0 \| 3] | - | Charge over-current alarm |
| `TempHigh` | 14 | 2 | Intel (1+) | (1, 0) | [0 \| 3] | - | Temperature high alarm |
| `TempLow` | 16 | 2 | Intel (1+) | (1, 0) | [0 \| 3] | - | Temperature low alarm |
| `SOC_Low` | 20 | 2 | Intel (1+) | (1, 0) | [0 \| 3] | - | Low SOC alarm level |

---

## 3. Cell Voltages Frames (`CELL_VOLT_FRAME_1`, `CELL_VOLT_FRAME_2`, `CELL_VOLT_FRAME_3`)
* **CAN ID Range:** `0x04F4` (Decimal: 1268 to 1270)
* **Sender:** `BMSMaster`
* **Cycle Time:** 100 ms

### CELL_VOLT_FRAME_1 (ID: 1268)
| Signal Name | Start Bit | Bit Length | Byte Order | Scaling (Factor, Offset) | Range | Unit | Description |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| `StartCell_1` | 0 | 8 | Intel (1+) | (1, 0) | [1 \| 255] | - | Starting cell index for frame |
| `Cell_01` to `Cell_07` | 8 to 56 | 8 each | Intel (1+) | (0.02, 2.0) | [2 \| 4.5] | V | Individual cell voltages (Cells 1–7) |

### CELL_VOLT_FRAME_2 (ID: 1269)
| Signal Name | Start Bit | Bit Length | Byte Order | Scaling (Factor, Offset) | Range | Unit | Description |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| `StartCell_8` | 0 | 8 | Intel (1+) | (1, 0) | [1 \| 255] | - | Starting cell index for frame |
| `Cell_08` to `Cell_14` | 8 to 56 | 8 each | Intel (1+) | (0.02, 2.0) | [2 \| 4.5] | V | Individual cell voltages (Cells 8–14) |

### CELL_VOLT_FRAME_3 (ID: 1270)
| Signal Name | Start Bit | Bit Length | Byte Order | Scaling (Factor, Offset) | Range | Unit | Description |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| `StartCell_15` | 0 | 8 | Intel (1+) | (1, 0) | [1 \| 255] | - | Starting cell index for frame |
| `Cell_15` to `Cell_21` | 8 to 56 | 8 each | Intel (1+) | (0.02, 2.0) | [2 \| 4.5] | V | Individual cell voltages (Cells 15–21) |

---

## 4. Cell Temperature Frame (`CELL_TEMP`)
* **CAN ID:** `0x05F4` (Decimal: 1524)
* **Sender:** `BMSMaster`
* **Cycle Time:** 100 ms

| Signal Name | Start Bit | Bit Length | Byte Order | Scaling (Factor, Offset) | Range | Unit | Description |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| `MaxCellTemp` | 0 | 8 | Intel (1+) | (1, -50) | [-50 \| 200] | °C | Maximum cell temperature |
| `MaxCtNO` | 8 | 8 | Intel (1+) | (1, 1) | [1 \| 250] | - | Index of the hottest cell sensor |
| `MinCellTemp` | 16 | 8 | Intel (1+) | (1, -50) | [-50 \| 200] | °C | Minimum cell temperature |
| `MinCtNO` | 24 | 8 | Intel (1+) | (1, 1) | [1 \| 250] | - | Index of the coldest cell sensor |
| `AvrgCellTem` | 32 | 8 | Intel (1+) | (1, -50) | [-50 \| 200] | °C | Average cell temperature |