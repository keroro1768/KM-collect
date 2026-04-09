# 測試覆蓋率矩陣 (Test Coverage Matrix)

> 本文件定義 M487_ScsiTool 專案各模組的測試覆蓋率目標。

---

## 📑 目錄

1. [覆蓋率目標](#覆蓋率目標)
2. [模組覆蓋率矩陣](#模組覆蓋率矩陣)
3. [覆蓋率度量標準](#覆蓋率度量標準)
4. [達成策略](#達成策略)

---

## 覆蓋率目標

### 整體目標

| 指標 | 當前目標 | 長期目標 |
|------|----------|----------|
| **行覆蓋率** | ≥ 80% | ≥ 90% |
| **分支覆蓋率** | ≥ 70% | ≥ 80% |
| **函式覆蓋率** | ≥ 90% | ≥ 95% |
| **檔案覆蓋率** | ≥ 85% | ≥ 90% |

### 分層目標

```
┌─────────────────────────────────────────┐
│   Application Layer    │  目標: 95%    │
├─────────────────────────────────────────┤
│   Protocol Layer        │  目標: 85%    │
├─────────────────────────────────────────┤
│   Driver Layer          │  目標: 80%    │
├─────────────────────────────────────────┤
│   HAL Layer             │  目標: 70%    │
└─────────────────────────────────────────┘
```

---

## 模組覆蓋率矩陣

### 核心模組

| 模組 | 檔案 | 行覆蓋率 | 分支覆蓋率 | 測試數量 | 優先級 |
|------|------|----------|------------|----------|--------|
| **I2C Driver** | i2c_driver.c | 95% | 90% | 25 | P1 |
| **USB MSC** | usb_msc.c | 90% | 85% | 40 | P1 |
| **SCSI Parser** | scsi_parser.c | 95% | 92% | 35 | P1 |
| **HID Parser** | hid_parser.c | 92% | 88% | 20 | P1 |
| **UART Debug** | uart_debug.c | 88% | 80% | 15 | P2 |
| **Buffer Mgr** | buffer.c | 98% | 95% | 30 | P1 |
| **FAT FS** | fatfs_port.c | 85% | 78% | 25 | P2 |
| **GPIO** | gpio_driver.c | 75% | 70% | 10 | P3 |

### 詳細模組矩陣

#### 1. I2C Driver

| 函式 | 測試狀態 | 行覆蓋率 | 備註 |
|------|----------|----------|------|
| `I2C_Init()` | ✅ | 100% | 初始化測試 |
| `I2C_DeInit()` | ✅ | 100% | 清理測試 |
| `I2C_WriteReg()` | ✅ | 98% | 含 NACK 重試 |
| `I2C_ReadReg()` | ✅ | 96% | 含逾時處理 |
| `I2C_WriteBlock()` | ✅ | 95% | 多位元組寫入 |
| `I2C_ReadBlock()` | ✅ | 92% | 多位元組讀取 |
| `I2C_Transfer()` | ✅ | 88% | 複合操作 |

#### 2. USB MSC

| 函式/狀態 | 測試狀態 | 行覆蓋率 | 備註 |
|-----------|----------|----------|------|
| `MSC_Init()` | ✅ | 100% | |
| `MSC_MassReset()` | ✅ | 95% | |
| `MSC_GetMaxLUN()` | ✅ | 100% | |
| `MSC_BulkIn()` | ✅ | 90% | |
| `MSC_BulkOut()` | ✅ | 88% | |
| `MSC_CBW_Parse()` | ✅ | 96% | |
| `MSC_CSW_Build()` | ✅ | 94% | |
| `MSC_BOT_State` | ✅ | 85% | 狀態機覆蓋 |

#### 3. SCSI Parser

| 命令 | 測試狀態 | 行覆蓋率 | 備註 |
|------|----------|----------|------|
| INQUIRY (0x12) | ✅ | 100% | |
| READ(10) (0x28) | ✅ | 98% | |
| WRITE(10) (0x2A) | ✅ | 97% | |
| TEST_UNIT_READY (0x00) | ✅ | 100% | |
| REQUEST_SENSE (0x03) | ✅ | 95% | |
| READ_CAPACITY(10) (0x25) | ✅ | 100% | |
| MODE_SENSE(10) (0x5A) | ✅ | 85% | |
| VERIFY(10) (0x2F) | ✅ | 80% | |

#### 4. HID Parser

| 函式 | 測試狀態 | 行覆蓋率 | 備註 |
|------|----------|----------|------|
| `HID_Parser_Init()` | ✅ | 100% | |
| `HID_GetReportID()` | ✅ | 100% | |
| `HID_ParseReport()` | ✅ | 95% | |
| `HID_ValidateReport()` | ✅ | 98% | |
| `HID_BuildReport()` | ✅ | 90% | |
| `HID_GetReportInfo()` | ✅ | 92% | |

#### 5. UART Debug

| 函式 | 測試狀態 | 行覆蓋率 | 備註 |
|------|----------|----------|------|
| `UART_Debug_Init()` | ✅ | 100% | |
| `UART_Log_Level()` | ✅ | 90% | |
| `UART_Log_Printf()` | ✅ | 88% | |
| `UART_Log_SetLevel()` | ✅ | 95% | |
| `UART_Log_Flush()` | ✅ | 80% | |

#### 6. Buffer Manager

| 函式 | 測試狀態 | 行覆蓋率 | 備註 |
|------|----------|----------|------|
| `Buffer_Create()` | ✅ | 100% | |
| `Buffer_Destroy()` | ✅ | 100% | |
| `Buffer_Write()` | ✅ | 98% | |
| `Buffer_Read()` | ✅ | 98% | |
| `Buffer_Clear()` | ✅ | 100% | |
| `Buffer_IsFull()` | ✅ | 100% | |
| `Buffer_IsEmpty()` | ✅ | 100% | |

---

## 覆蓋率度量標準

### 測試數量目標

| 模組 | 單元測試 | 整合測試 | 總計 |
|------|----------|----------|------|
| I2C Driver | 25 | 5 | 30 |
| USB MSC | 40 | 10 | 50 |
| SCSI Parser | 35 | 8 | 43 |
| HID Parser | 20 | 5 | 25 |
| Buffer Manager | 30 | 0 | 30 |
| FAT FS | 25 | 8 | 33 |
| **總計** | **175** | **36** | **211** |

### 測試類型分布

```
單元測試 (175)
├── Happy Path Tests (80)     46%
├── Error Handling Tests (45)  26%
├── Boundary Tests (30)       17%
└── Edge Case Tests (20)      11%

整合測試 (36)
├── Module Integration (15)    42%
├── Protocol Compliance (12)   33%
└── Performance Tests (9)      25%
```

---

## 達成策略

### 覆蓋率提升步驟

#### Phase 1: 基礎覆蓋（P1 模組）

```
目標：行覆蓋率 ≥ 80%

行動：
1. 為所有 P1 模組建立基礎測試
2. 確保核心函式 100% 覆蓋
3. 補齊錯誤處理路徑測試
```

#### Phase 2: 深度覆蓋

```
目標：分支覆蓋率 ≥ 70%

行動：
1. 識別未覆蓋的分支
2. 增加邊界條件測試
3. Mock 依賴以隔離測試
```

#### Phase 3: 維護與改進

```
目標：保持並提升覆蓋率

行動：
1. 新功能 TDD
2. 重構時保持覆蓋率
3. 定期審查測試品質
```

### 覆蓋率不足時的處理

```cpp
// 使用註解標記預期但暫時無法達到的覆蓋率
// LCOV_EXCL_START - 硬體特定錯誤路徑，難以在離線環境觸發
void I2C_HardwareError_Handler(void) {
    // This is only triggered by actual hardware failure
    // Tested manually on target hardware
    NVIC_SystemReset();
}
// LCOV_EXCL_STOP
```

---

## 延伸閱讀

- [M487 測試策略](./M487_Testing_Strategy.md)
- [CI 整合](./CI_Integration.md)
- [Unity 最佳實踐](../Unity/Best_Practices.md)

---
