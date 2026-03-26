# USB-I3C Bridge 系統架構詳細說明

> 更新日期: 2026-03-26
> 版本: v1.0

---

## 1. 系統整體架構

```
┌─────────────────────────────────────────────────────────────────────────┐
│                         USB-I3C Bridge 系統                             │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                         │
│  ┌──────────────────────┐         ┌──────────────────────┐            │
│  │   Host PC (Windows)  │         │  I3C Target Devices   │            │
│  │                      │         │                      │            │
│  │  ┌────────────────┐  │   I3C   │  ┌────────────────┐  │            │
│  │  │ Test Tool App  │◄─┼─────────┼─►│  Sensor #1     │  │            │
│  │  │ (Vendor Cmd)   │  │   Bus   │  │  (e.g., LSM6DSO)│  │            │
│  │  └────────────────┘  │         │  └────────────────┘  │            │
│  │                      │         │                      │            │
│  │  ┌────────────────┐  │         │  ┌────────────────┐  │            │
│  │  │ USB Storage    │◄─┼─────────┼─►│  Sensor #2     │  │            │
│  │  │ (Virtual Drive)│  │         │  │  (e.g., LPS22HH)│  │            │
│  │  └────────────────┘  │         │  └────────────────┘  │            │
│  │                      │         │                      │            │
│  │  ┌────────────────┐  │         │  ┌────────────────┐  │            │
│  │  │ Serial Console │◄─┼─────────┼─►│  Sensor #N     │  │            │
│  │  │ (CDC ACM)      │  │         │  │  (... )         │  │            │
│  │  └────────────────┘  │         │  └────────────────┘  │            │
│  └──────────────────────┘         └──────────────────────┘            │
│              │                                    │                    │
│              │ USB                                │                    │
│              │ (Composite Device)                 │                    │
│              ▼                                    ▼                    │
│  ┌─────────────────────────────────────────────────────────┐          │
│  │                   STM32H563ZI Bridge                    │          │
│  │  ┌───────────────────────────────────────────────────┐  │          │
│  │  │            USB Composite Device                   │  │          │
│  │  │  ┌─────────────┬─────────────┬───────────────┐   │  │          │
│  │  │  │ Mass Storage│   CDC ACM   │  Vendor Specific│  │  │          │
│  │  │  │ (MSC)       │ (Serial)    │  (Custom HID)  │   │  │          │
│  │  │  │ Drive: D:   │ COM Port    │  Control EP    │   │  │          │
│  │  │  └─────────────┴─────────────┴───────────────┘   │  │          │
│  │  └───────────────────────────────────────────────────┘  │          │
│  │                          │                               │          │
│  │                    ┌─────┴─────┐                         │          │
│  │                    │  Bridge   │                         │          │
│  │                    │  Core     │                         │          │
│  │                    │  (Firmware)                        │          │
│  │                    └─────┬─────┘                         │          │
│  │                          │                               │          │
│  │         ┌────────────────┼────────────────┐             │          │
│  │         │                │                │             │          │
│  │         ▼                ▼                ▼             │          │
│  │  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐   │          │
│  │  │   I3C       │  │    I2C      │  │   GPIO      │   │          │
│  │  │  Controller │  │  Controller │  │  Control    │   │          │
│  │  │  (Primary) │  │  (Fallback) │  │  (LED/BTN)  │   │          │
│  │  └─────────────┘  └─────────────┘  └─────────────┘   │          │
│  └─────────────────────────────────────────────────────────┘          │
└─────────────────────────────────────────────────────────────────────────┘
```

---

## 2. USB Composite Device 設計

### 2.1 介面配置

| 介面 | Class | 功能 | 用途 |
|------|-------|------|------|
| **Interface 0** | MSC (Mass Storage) | 虛擬磁碟 (如 D:) | 存放設定檔、韌體更新、資料 Log |
| **Interface 1** | CDC ACM | 虛擬序列埠 | 除錯訊息、命令列介面 |
| **Interface 2** | Vendor Specific | 自定義 HID | 高速資料傳輸、控制指令 |

### 2.2 MSC (USB Storage) 設計

**用途:**
- 存放配置檔 (config.json)
- 存放韌體更新檔 (.bin)
- 匯出資料日誌
- 作為測試工具的訪問入口

**實現:**
```c
// USB MSC 結構
typedef struct {
    uint8_t media[16 * 1024];  // 16KB 虛擬儲存
    uint32_t media_size;
} USBD_StorageTypeDef;

// 檔案系統結構
// D:\
// ├── config.txt        # 設定檔
// ├── firmware.bin     # 韌體更新
// ├── log.txt          # 操作日誌
// └── devices.json     # I3C 裝置列表
```

**優點:**
- Windows/Linux 內建支援，無需驅動程式
- 可用檔案總管直接存取
- 適合非技術人員操作

### 2.3 CDC ACM (Virtual COM Port) 設計

**用途:**
- 除錯訊息輸出
- 文字命令列介面
- 讀取診斷資訊

**命令範例:**
```
> scan
I3C: Found 2 devices
  - 0x32: LSM6DSO
  - 0x5D: LPS22HH

> read 0x32 0x0F 2
Data: [A5 3C]

> write 0x5D 0x10 0x80
OK
```

### 2.4 Vendor Specific (Custom HID) 設計

**用途:**
- 高速資料傳輸
- Vendor Command 發送
- 即時控制

**Report Descriptor:**
```c
#define VENDOR_CMD_REPORT_SIZE 64

static uint8_t vendor_report_desc[] = {
    0x06, 0x00, 0xFF,     // Usage Page (Vendor)
    0x09, 0x01,           // Usage (Vendor 1)
    0xA1, 0x01,           // Collection (Application)
    0x09, 0x01,           // Usage (Vendor 1)
    0x15, 0x00,           // Logical Minimum (0)
    0x26, 0xFF, 0x00,     // Logical Maximum (255)
    0x75, 0x08,           // Report Size (8)
    0x95, 0x40,           // Report Count (64)
    0x81, 0x02,           // Input (Data)
    0x09, 0x02,           // Usage (Vendor 2)
    0x91, 0x02,           // Output (Data)
    0xC0                 // End Collection
};
```

---

## 3. I3C / I2C 介面設計

### 3.1 I3C Controller (Primary)

**功能:**
- 作為 I3C Bus 主控
- 支援動態定址 (ENTDAA)
- 支援 Hot-Join
- 支援 In-Band Interrupt (IBI)

**支援模式:**
- Polling Mode
- Interrupt Mode
- DMA Mode

### 3.2 I2C Controller (Fallback)

**功能:**
- 支援 legacy I2C 裝置
- 與 I3C 共用匯流排

**參數:**
- Speed: Standard (100kHz) / Fast (400kHz) / Fast+ (1MHz)
- Address: 7-bit / 10-bit

---

## 4. Host PC 測試工具

### 4.1 工具選擇

| 工具 | 類型 | 適用介面 | 說明 |
|------|------|----------|------|
| **BridgeCLI** | Command Line | CDC ACM | 文字介面測試 |
| **BridgeTool** | GUI Application | Vendor HID | 圖形化測試 |
| **BridgeStorage** | File Access | MSC | 檔案介面測試 |
| **libusb** | Library | Vendor HID | 自定義開發 |

### 4.2 BridgeCLI (CDC ACM)

```bash
# 連接到序列埠 (COM3 或 /dev/ttyUSB0)
$ putty -serial COM3 -baud 115200

# 指令範例
scan i2c           # 掃描 I2C  bus
scan i3c           # 掃描 I3C  bus
read <addr> <reg> [len]
write <addr> <reg> <data>
reset
status
help
```

### 4.3 BridgeTool (GUI + Vendor HID)

**功能:**
- 圖形化介面
- 支援 Vendor Command
- 支援巨集腳本
- 資料視覺化

**畫面配置:**
```
┌─────────────────────────────────────────┐
│     USB-I3C Bridge Control Tool         │
├─────────────────────────────────────────┤
│  [Device: Connected] [COM3]            │
├─────────────────────────────────────────┤
│  Bus: ○ I3C  ● I2C                      │
│  Address: [0x32 ▼]                      │
│  Register: [0x00    ] [Length: 1]       │
├─────────────────────────────────────────┤
│  Read  │  Write  │  Scan  │  Custom    │
├─────────────────────────────────────────┤
│  Response:                              │
│  [00 3F A5 8C ...                      │
│  ]                                      │
└─────────────────────────────────────────┘
```

### 4.4 BridgeStorage (MSC)

```
D:\> type config.txt
# USB-I3C Bridge Configuration
# Format: key=value

i3c_baudrate=3000000
i2c_baudrate=400000
default_device=0x32

D:\> echo "read 0x32 0x0F 2" > command.txt
D:\> type response.txt
```

---

## 5. 完整通訊流程

### 5.1 Host → Bridge (Write)

```
┌──────────┐    Vendor HID     ┌──────────┐    I3C Bus     ┌──────────┐
│  Host PC │ ────────────────► │  Bridge  │ ─────────────► │  I3C     │
│ Tool     │   Report (64B)    │  Firmware│  Write Cmd     │  Target  │
└──────────┘                   └──────────┘                └──────────┘

Report Format:
[0] 0x01          - Command: Write
[1] 0x32          - I3C Target Address (dynamic)
[2] 0x0F          - Register Address
[3] 0x02          - Data Length
[4] 0xA5          - Data[0]
[5] 0x3C          - Data[1]
[6-63] 0x00       - Reserved
```

### 5.2 Host → Bridge (Read)

```
┌──────────┐    Vendor HID     ┌──────────┐    I3C Bus     ┌──────────┐
│  Host PC │ ────────────────► │  Bridge  │ ─────────────► │  I3C     │
│ Tool     │   Report (64B)    │  Firmware│  Read Cmd      │  Target  │
└──────────┘                   └──────────┘                └──────────┘
                                ▼
                         ┌──────────┐
                         │ Response │
                         │ Report   │
                         └──────────┘
                                │
                                ▼
                         ┌──────────┐    Vendor HID     ┌──────────┐
                         │  Bridge  │ ◄──────────────── │  Host PC │
                         │  Firmware│   Report (64B)    │  Tool    │
                         └──────────┘                   └──────────┘

Request Report:
[0] 0x02          - Command: Read
[1] 0x32          - I3C Target Address
[2] 0x0F          - Register Address
[3] 0x02          - Data Length
[4-63] 0x00       - Reserved

Response Report:
[0] 0x82          - Command: Read Response
[1] 0x02          - Actual Data Length
[2] 0xA5          - Data[0]
[3] 0x3C          - Data[1]
[4-63] 0x00       - Reserved
```

### 5.3 MSC 方式存取

```
Host                          Bridge
────                          ──────
Write config.txt ──────────► Parse & Apply
                                │
Read log.txt    ◄──────────── Export to file
```

---

## 6. I3C CLIENT 端設計

### 6.1 角色說明

在 USB-I3C Bridge 系統中：

| 角色 | 硬體 | 功能 |
|------|------|------|
| **I3C Controller** | STM32H563ZI | 主動控制 I3C bus，發送命令 |
| **I3C Target** | 其他 I3C 裝置 | 從屬裝置，回應 Controller |

### 6.2 I3C Target 端 (Client)

若需要在另一個開發板上实现 I3C Target：

```c
// I3C Target Mode Example
void I3C_Target_Init(void) {
    hi3c1.Instance = I3C1;
    hi3c1.Init.Mode = I3C_TARGET_MODE;
    hi3c1.Init.SlaveAddress = 0x32;  // 靜態位址
    
    HAL_I3C_Init(&hi3c1);
}

// 當作 I3C Target 回應
void HAL_I3C_TgtMatchCallback(I3C_HandleTypeDef *hi3c, uint8_t *payload, uint16_t size) {
    // 處理來自 Controller 的命令
}

void HAL_I3C_TgtWriteCallback(I3C_HandleTypeDef *hi3c, uint8_t *buf, uint16_t size) {
    // 處理寫入資料
}

void HAL_I3C_TgtReadCallback(I3C_HandleTypeDef *hi3c, uint8_t *buf, uint16_t size) {
    // 提供讀取資料
    buf[0] = sensor_data;
}
```

---

## 7. 系統功能對照表

| 功能 | MSC | CDC ACM | Vendor HID |
|------|-----|---------|------------|
| 配置讀寫 | ✓ | ✓ | ✓ |
| 韌體更新 | ✓ | - | ✓ |
| 即時控制 | - | ✓ | ✓ |
| 資料傳輸 | - | ✓ | ✓ |
| 除錯訊息 | - | ✓ | - |
| 無驅動程式 | ✓ | ✓ | - |
| 高速傳輸 | - | - | ✓ |
| 相容性 | ✓✓ | ✓ | ✓ |

---

## 8. 開發優先順序

### Phase 1: 基本通訊 (MVP)
1. USB CDC ACM + I3C 讀寫
2. 文字命令列介面
3. I3C 基本功能驗證

### Phase 2: 強化功能
4. USB MSC 介面
5. 配置文件支援
6. 韌體更新機制

### Phase 3: 測試工具
7. BridgeTool GUI
8. Vendor HID 支援
9. 巨集腳本功能

### Phase 4: Client 端
10. I3C Target 端程式碼
11. 雙向通訊測試

---

## 9. 檔案結構

```
D:\Project\
├── USB-I3C-Bridge\
│   ├── firmware\
│   │   ├── Core\           # 核心驅動
│   │   ├── USB\            # USB 介面
│   │   ├── I3C\            # I3C 介面
│   │   └── I2C\            # I2C 介面
│   ├── tools\
│   │   ├── BridgeCLI\      # 命令列工具
│   │   ├── BridgeTool\     # GUI 工具
│   │   └── BridgeStorage\ # 儲存介面工具
│   ├── client\
│   │   └── I3C-Target\     # Client 端範例
│   └── docs\
│       └── Architecture.md # 本文件
└── KM-collect\
    └── Hardware\
        └── I3C-USB-Bridge\
            ├── 00-Project-Tasks.md
            ├── 06-USB-I3C-Bridge-Firmware.md
            ├── 07-System-Architecture.md
            └── 08-Reference-Projects.md   # 現有專案參考
```

---

## 10. 現有開源專案參考

### 10.1 I2C Tiny USB (最相關!)

**專案:** https://github.com/harbaum/I2C-Tiny-USB

**說明:**
- 經典的 USB → I2C adapter，基於 ATtiny45
- 提供 Linux kernel driver (`i2c-tiny-usb`)
- 讓 I2C bus 直接出現在 Linux i2c subsystem
- 支援 Windows/Mac/Linux

**特色:**
- 完全開源 (硬+軟)
- 使用 libusb 通訊
- 可被 `lm_sensors` 直接使用

**硬體:**
- ATtiny45 (4KB Flash)
- 12MHz crystal
- USB Soft AP

**編譯:**
```bash
git clone https://github.com/harbaum/I2C-Tiny-USB
cd I2C-Tiny-USB/firmware
make -f Makefile-avrusb.tiny45
```

### 10.2 i2c-star (STM32F103 版本)

**專案:** https://github.com/daniel-thompson/i2c-star

**說明:**
- 基於 STM32F103 ("Blue Pill")
- 類似 i2c-tiny-usb 但使用更強大的 MCU
- 支援更高速的 I2C

### 10.3 Linux Kernel I3C Subsystem

**文件:** https://docs.kernel.org/driver-api/i3c/index.html

**說明:**
- Linux 官方 I3C driver API
- I3C master controller driver API
- I3C device driver API
- 涵蓋完整 I3C 協定說明

### 10.4 CH341 USB to I2C

**專案:** https://github.com/gschorcht/i2c-ch341-usb

**說明:**
- Linux kernel driver for CH341
- CH341 是常見的 USB to I2C 晶片

### 10.5 商業產品參考

| 產品 | 說明 | 連結 |
|------|------|------|
| **Binho I3C Target Adapter** | USB I3C adapter | binho.io |
| **MCCI Model 2712** | SuperMITT I3C/I2C Adapter | store.mcci.com |
| **PGY-I3C USB Adapter** | I3C Protocol Analyzer | prodigytechno.com |
| **CP2112** | USB to SMBus (I2C) | Silicon Labs |

---

## 11. 總結

本系統提供三種 USB 介面：
1. **MSC (Storage)** - 適合檔案操作、設定管理
2. **CDC ACM (Serial)** - 適合除錯、簡單命令
3. **Vendor HID** - 適合高速控制、Vendor Command

測試工具對應：
- BridgeCLI → CDC ACM
- BridgeTool → Vendor HID
- BridgeStorage → MSC

完整的 I3C CLIENT 端程式碼可獨立開發，作為日後擴展使用。

### 重要參考
- **i2c-tiny-usb** 是最佳參考範本，實現了類似的 USB→I2C 架構
- 可研究其 Linux driver 設計，移植到 I3C
- 或參考其 libusb 通訊方式實現 Host Tool
