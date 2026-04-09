# HID over I2C 深度技術規範 (基於 Microsoft Protocol Spec V1.0)

本文件詳細記錄了 HID over I2C 的底層通訊協議，包含暫存器定義、指令集、數據包格式以及電源管理機制。

---

## 1. 暫存器映射 (Register Map)

HID over I2C 設備必須暴露以下關鍵暫存器，用於主機 (HOST) 與設備 (DEVICE) 之間的控制與數據交換。

### 1.1 核心暫存器定義

| 暫存器名稱 | 偏移量/索引 | 權限 | 說明 |
| :--- | :--- | :--- | :--- |
| **HID Descriptor Register** | `wReportDescRegister` | 讀 | 儲存 HID 描述符的起始地址。 |
| **Input Register** | `wInputRegister` | 讀 | 用於讀取輸入報告 (Input Report)。設備有數據時會觸發中斷。 |
| **Output Register** | `wOutputRegister` | 寫 | 用於發送輸出報告 (Output Report) 給設備。 |
| **Command Register** | `wCommandRegister` | 寫 | 用於發送控制指令 (OPCODE) 給設備。 |
| **Data Register** | `wDataRegister` | 讀/寫 | 用於交換與指令相關的數據 (如 GET/SET Report 的 payload)。 |

### 1.2 指令暫存器 (Command Register) 格式
指令暫存器為 2 Byte 結構：

- **High Byte (高位元組)**:
  - `[7:4]`: 保留 (Reserved)，必須為 `0000b`。
  - `[3:0]`: **OpCode (操作碼)**。定義具體指令。
- **Low Byte (低位元組)**:
  - `[7:6]`: 保留 (Reserved)，必須為 `00b`。
  - `[5:4]`: **Report Type (報告類型)**。
    - `01b`: Input
    - `10b`: Output
    - `11b`: Feature
  - `[3:0]`: **Report ID**。對於簡單設備為 `0`；複雜設備 (多 TLC) 使用此欄位區分報告。若 ID $\ge 15$，則此處設為 `1111b` 且在指令後追加第三個 Byte。

---

## 2. 指令集 (Class Specific Requests)

| OpCode (Hex) | 指令名稱 | 方向 | 必備性 | 說明 |
| :--- | :--- | :--- | :--- | :--- |
| `0x1` | **RESET** | HOST $\rightarrow$ DEV | 必須 | 重置設備至初始化狀態。設備重置後需在 Input Register 寫入 `0x0000` 並觸發中斷。 |
| `0x2` | **GET_REPORT** | HOST $\leftrightarrow$ DEV | 必須* | 讀取特定類型的報告 (Input/Feature)。數據存放在 Data Register。 |
| `0x3` | **SET_REPORT** | HOST $\rightarrow$ DEV | 必須* | 設定特定類型的報告 (Output/Feature)。數據寫入 Data Register。 |
| `0x4` | **GET_IDLE** | HOST $\leftrightarrow$ DEV | 選配 | 讀取設備的空閒報告率 (Reporting Frequency)。 |
| `0x5` | **SET_IDLE** | HOST $\rightarrow$ DEV | 選配 | 設定設備的空閒報告率。 |
| `0x6` | **GET_PROTOCOL**| HOST $\leftrightarrow$ DEV | 選配 | 查詢當前協議模式 (Boot 或 Report)。 |
| `0x7` | **SET_PROTOCOL**| HOST $\rightarrow$ DEV | 選配 | 設定協議模式。 |
| `0x8` | **SET_POWER** | HOST $\rightarrow$ DEV | 設備必備 | 設定電源狀態 (`00`: ON, `01`: SLEEP)。 |

*\* 僅在設備包含該類型報告時為必須。*

---

## 3. 數據傳輸格式 (Data Format)

### 3.1 數據暫存器 (Data Register) 封裝
所有透過 Data Register 交換的報告（Input, Output, Feature）都必須遵循以下格式：

- **長度欄位 (Length)**: 前 2 個 Byte，代表整個數據包的總長度（含這 2 個長度 Byte）。
- **報告內容 (Payload)**: 隨後的數據。若定義了 Report ID，則 Payload 的第一個 Byte 為 Report ID。

**範例**：若一個報告內容為 4 Bytes，則 Data Register 總長度為 $2 + 4 = 6$ Bytes。前兩個 Byte 儲存 `0x0006`。

### 3.2 輸入報告 (Input Report) 流程
1. **DEVICE** $\rightarrow$ **HOST**: 設備有數據 $\rightarrow$ 拉低/高 **Interrupt** 線。
2. **HOST** $\rightarrow$ **DEVICE**: 讀取 `wInputRegister` $\rightarrow$ 獲取 `[Length (2B)] + [Report Payload]`。
3. **DEVICE** $\rightarrow$ **HOST**: 數據讀完後 $\rightarrow$ 釋放 **Interrupt** 線。

---

## 4. 電源管理 (Power Management)

### 4.1 HIPO (Host Initiated Power Optimizations)
主機控制設備進入低功耗狀態：
- **ON**: 設備正常運作，必須在 1 秒內完成狀態切換。
- **SLEEP**: 設備進入低功耗模式，必須釋放中斷線。但設備仍可透過中斷線發送「喚醒」信號給主機。

### 4.2 DIPO (Device Initiated Power Optimizations)
設備自主優化：
- 設備可根據內部狀態 (如加速度計檢測到靜止) 自行降低採樣頻率或進入低功耗模式。
- 必須對主機透明，且不能導致數據丟失。

---

## 5. ACPI 定義 (ACPI Integration)

設備必須在 ACPI 中定義，以便主機在啟動時發現。關鍵屬性包括：
- `_HID`: 設備 ID (例如 `MSFT0011`)。
- `_CID`: 兼容 ID (必須為 `PNP0C50`)。
- `_CRS`: 資源設置，包含 `I2CSerialBus` (地址) 和 `GpioInt` (中斷線)。
- `_DSM`: 設備特定方法，用於返回 **HID Descriptor Address** (2 Bytes)。

---

## 6. 實作範例：加速度計 (Accelerometer)
- **HID Descriptor**: 定義 `wMaxInputLength = 11` (2B 長度 + 9B 數據)。
- **Input Report (9 Bytes)**:
  - Byte 0: Sensor State (狀態)
  - Byte 1: Sensor Event (事件)
  - Byte 2-3: X-Axis Acceleration (16-bit)
  - Byte 4-5: Y-Axis Acceleration (16-bit)
  - Byte 6-7: Z-Axis Acceleration (16-bit)
  - Byte 8: Motion Intensity (運動強度)
