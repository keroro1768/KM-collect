# USB to I3C Bridge Firmware 開發指南

> 建立日期: 2026-03-26  
> 目標硬體: STM32H563ZI (NUCLEO-H563ZI)  
> 最終目標: USB 轉 I3C Bridge 韌體

---

## 1. 開發工具鏈 (CLI)

### 選項 A: PlatformIO (推薦)

**優點:**
- 跨平台 (Windows/Linux/macOS)
- 內建建置系統，無需手動設定
- 支援多種 STM32 系列
- CLI 操作

**安裝:**
```bash
# 安裝 PlatformIO
pip install platformio

# 建立 STM32H5 專案
pio init --board STM32H563ZIT6 --project-dir USB-I3C-Bridge
cd USB-I3C-Bridge
```

**編譯與燒錄:**
```bash
# 編譯
pio run

# 燒錄 (使用 ST-Link)
pio run --target upload

# 監控序列埠
pio device monitor
```

### 選項 B: STM32CubeMX + GCC Makefile

**優點:**
- 官方支援
- 可生成 Makefile 專案

**設定:**
1. 在 STM32CubeMX 中選擇 NUCLEO-H563ZI
2. 配置所需的週邊 (USB, I3C, I2C)
3. Project > Settings > Toolchain / IDE: Makefile
4. 生成專案後使用:

```bash
# 編譯
make -j4

# 燒錄 (需要 st-link CLI 或 openocd)
# 或使用 STM32CubeProgrammer CLI
STM32_Programmer_CLI -c port=SWD -w build/project.bin 0x08000000
```

### 選項 C: 開源 toolchain (手動設定)

**所需工具:**
- ARM GCC Toolchain: https://developer.arm.com/downloads/-/gnu-rm
- ST-Link tools: https://github.com/stlink-org/stlink
- (可選) OpenOCD

---

## 2. USB HID 範例程式碼

### 2.1 USB HID 基本結構

以下基於 STM32CubeH5 的 USB HID 範例:

```c
// Core/Inc/usbd_customhid.h 結構
typedef struct {
  uint8_t ReportID;
  uint8_t ReportData[MAX_CUSTOMHID_DATA_LEN - 1];
} CUSTOMHID_ReportTypeDef;

typedef struct _USBD_CUSTOMHID_ItfTypeDef {
  int8_t (*Edit_Report)(uint8_t *report, uint16_t *len);
} USBD_CUSTOMHID_ItfTypeDef;
```

### 2.2 自定義 HID 報告描述符

```c
// USB 報告描述符 - 可自定義傳輸格式
__ALIGN_BEGIN static uint8_t CUSTOM_HID_ReportDesc[CUSTOM_HID_REPORT_DESC_SIZE] __ALIGN_END = {
    0x06, 0x00, 0xFF,  // Usage Page (Vendor Defined)
    0x09, 0x01,        // Usage (Vendor Page 1)
    0xA1, 0x01,        // Collection (Application)
    0x09, 0x01,        // Usage (Vendor Page 1)
    0x15, 0x00,        // Logical Minimum (0)
    0x26, 0xFF, 0x00, // Logical Maximum (255)
    0x75, 0x08,        // Report Size (8 bits)
    0x95, 0x40,        // Report Count (64 bytes)
    0x81, 0x02,        // Input (Data, Variable, Absolute)
    0x09, 0x02,        // Usage (Vendor Page 1)
    0x15, 0x00,        // Logical Minimum (0)
    0x26, 0xFF, 0x00, // Logical Maximum (255)
    0x75, 0x08,        // Report Size (8 bits)
    0x95, 0x40,        // Report Count (64 bytes)
    0x91, 0x02,        // Output (Data, Variable, Absolute)
    0xC0               // End Collection
};
```

### 2.3 USB HID 資料傳輸

```c
// Inc/usbd_customhid_if.h
extern USBD_CUSTOMHID_ItfTypeDef USBD_CUSTOMHID_fops;

// 發送資料到 PC
uint8_t USB_SendData(uint8_t *data, uint16_t len) {
    return USBD_CUSTOMHID_SendReport(&hUsbDeviceFS, data, len);
}

// 接收來自 PC 的資料 (Callback)
static int8_t CustomHID_ReceiveReport(uint8_t *report, uint16_t *len) {
    // 處理接收到的資料
    // report[0] = ReportID
    // report[1..*len] = 資料
    return USBD_OK;
}
```

### 2.4 STM32H5 USB HID Composite (CDC + HID)

**參考:** https://github.com/stm32-hotspot/USBD_Composite_STM32H503

```c
// Main functions for CDC + HID
int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USB_Device_Init();
    MX_I3C1_Init();
    
    while (1) {
        // 處理 USB 資料
        // 處理 I3C 通訊
    }
}
```

---

## 3. I2C 範例程式碼 (STM32H5 HAL)

### 3.1 I2C 初始化

```c
// I2C Polling Mode - Master Write
void MX_I2C1_Init(void) {
    hi2c1.Instance = I2C1;
    hi2c1.Init.Timing = 0x10909CEC;  // 100kHz @ 250MHz
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    
    if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
        Error_Handler();
    }
}
```

### 3.2 I2C Polling 模式

```c
// Master Polling Write
HAL_StatusTypeDef I2C_WriteReg(uint16_t DevAddress, uint16_t MemAddress, uint8_t *data, uint16_t size) {
    return HAL_I2C_Mem_Write(&hi2c1, DevAddress << 1, MemAddress, 
                            I2C_MEMADD_SIZE_8BIT, data, size, HAL_MAX_DELAY);
}

// Master Polling Read
HAL_StatusTypeDef I2C_ReadReg(uint16_t DevAddress, uint16_t MemAddress, uint8_t *data, uint16_t size) {
    return HAL_I2C_Mem_Read(&hi2c1, DevAddress << 1, MemAddress, 
                           I2C_MEMADD_SIZE_8BIT, data, size, HAL_MAX_DELAY);
}

// I2C Scanner - 掃描所有裝置
void I2C_Scan(void) {
    printf("I2C Scanner:\r\n");
    for (uint8_t addr = 1; addr < 128; addr++) {
        if (HAL_I2C_IsDeviceReady(&hi2c1, addr << 1, 2, 10) == HAL_OK) {
            printf("Found: 0x%02X\r\n", addr);
        }
    }
}
```

### 3.3 I2C Interrupt 模式

```c
// 初始化後啟用中斷
void MX_I2C1_Init_IT(void) {
    MX_I2C1_Init();
    
    // 啟用中斷
    HAL_NVIC_SetPriority(I2C1_EV_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
    HAL_NVIC_SetPriority(I2C1_ER_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
}

// Interrupt Mode Write
HAL_StatusTypeDef I2C_WriteReg_IT(uint16_t DevAddress, uint16_t MemAddress, uint8_t *data, uint16_t size) {
    return HAL_I2C_Mem_Write_IT(&hi2c1, DevAddress << 1, MemAddress,
                                I2C_MEMADD_SIZE_8BIT, data, size);
}

// Interrupt Mode Read
HAL_StatusTypeDef I2C_ReadReg_IT(uint16_t DevAddress, uint16_t MemAddress, uint8_t *data, uint16_t size) {
    return HAL_I2C_Mem_Read_IT(&hi2c1, DevAddress << 1, MemAddress,
                               I2C_MEMADD_SIZE_8BIT, data, size);
}

// 中斷處理 (在 stm32h5xx_it.c 或 Callback 中)
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c) {
    // 讀取完成
}

void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c) {
    // 寫入完成
}
```

### 3.4 I2C DMA 模式

```c
// DMA 模式初始化
void MX_I2C1_Init_DMA(void) {
    MX_I2C1_Init();
    
    // 設定 DMA
    hdma_i2c1_rx.Instance = DMA1_Stream0;
    hdma_i2c1_rx.Init.Request = DMA_REQUEST_I2C1_RX;
    hdma_i2c1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_i2c1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_i2c1_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_i2c1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_i2c1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_i2c1_rx.Init.Mode = DMA_NORMAL;
    hdma_i2c1_rx.Init.Priority = DMA_PRIORITY_LOW;
    
    __HAL_LINKDMA(&hi2c1, hdmarx, hdma_i2c1_rx);
    HAL_DMA_Init(&hdma_i2c1_rx);
}

// DMA Write/Read
HAL_StatusTypeDef I2C_WriteReg_DMA(uint16_t DevAddress, uint16_t MemAddress, uint8_t *data, uint16_t size) {
    return HAL_I2C_Mem_Write_DMA(&hi2c1, DevAddress << 1, MemAddress,
                                 I2C_MEMADD_SIZE_8BIT, data, size);
}
```

---

## 4. I3C 範例程式碼 (基於 AN5879)

### 4.1 I3C 初始化

```c
// I3C Controller 初始化
void MX_I3C1_Init(void) {
    hi3c1.Instance = I3C1;
    hi3c1.Init.BaudRate = 3000000;  // 3 MHz
    hi3c1.Init.I3COnlyMode = I3C_ONLY_MODE_ENABLE;
    
    if (HAL_I3C_Init(&hi3c1) != HAL_OK) {
        Error_Handler();
    }
}
```

### 4.2 I3C 動態定址 (ENTDAA)

```c
// Target Descriptor
typedef struct {
    char *TARGET_NAME;
    uint32_t TARGET_ID;
    uint64_t TARGET_BCR_DCR_PID;
    uint8_t STATIC_ADDR;
    uint8_t DYNAMIC_ADDR;
} TargetDesc_TypeDef;

// ENTDAA - Enter Dynamic Address Assignment
HAL_StatusTypeDef I3C_AssignDynamicAddress(TargetDesc_TypeDef *target) {
    uint64_t payload;
    HAL_StatusTypeDef status;
    
    do {
        status = HAL_I3C_Ctrl_DynAddrAssign(&hi3c1, &payload, I3C_RSTDAA_THEN_ENTDAA, 5000);
        if (status == HAL_BUSY) {
            // 找到裝置，設定位址
            target->TARGET_BCR_DCR_PID = payload;
            HAL_I3C_Ctrl_SetDynAddr(&hi3c1, target->DYNAMIC_ADDR);
        }
    } while (status == HAL_BUSY);
    
    return status;
}
```

### 4.3 I3C 通訊

```c
// I3C Write (Private Write)
HAL_StatusTypeDef I3C_WriteReg(uint8_t DynAddr, uint8_t reg, uint8_t *data, uint16_t size) {
    uint32_t ctrlBuf;
    uint8_t txBuf[256];
    
    txBuf[0] = reg;
    memcpy(&txBuf[1], data, size);
    
    return HAL_I3C_Ctrl_Transmit(&hi3c1, DynAddr, txBuf, size + 1, 1000);
}

// I3C Read (Private Read)
HAL_StatusTypeDef I3C_ReadReg(uint8_t DynAddr, uint8_t reg, uint8_t *data, uint16_t size) {
    uint32_t ctrlBuf;
    uint8_t txBuf = reg;
    
    return HAL_I3C_Ctrl_Receive(&hi3c1, DynAddr, data, size, 1000);
}
```

---

## 5. USB 轉 I3C Bridge 韌體架構

### 5.1 系統架構

```
┌─────────────────────────────────────────────────────────┐
│                    STM32H563ZI                          │
├─────────────────────────────────────────────────────────┤
│  USB HID 介面              │  I3C Controller 介面       │
│  (64-byte reports)         │  (連接 I3C sensors)         │
├─────────────────────────────────────────────────────────┤
│                     資料轉發核心                         │
│  • USB → I3C: 解析 HID report，轉發到 I3C bus          │
│  • I3C → USB: 讀取 I3C 裝置，回傳 HID report          │
└─────────────────────────────────────────────────────────┘
```

### 5.2 通訊協定

**USB HID Report 格式 (64 bytes):**

| Offset | 大小 | 說明 |
|--------|------|------|
| 0 | 1 | Command ID |
| 1 | 1 | I3C Target Dynamic Address |
| 2 | 1 | Register Address / Command |
| 3 | 1 | Data Length |
| 4-63 | 60 | Data Payload |

**Command ID:**
- 0x01: I3C Write
- 0x02: I3C Read
- 0x03: I3C Scan / Get Device List
- 0x04: I2C Write (fallback)
- 0x05: I2C Read (fallback)

### 5.3 核心程式碼框架

```c
// main.c 框架
int main(void) {
    HAL_Init();
    SystemClock_Config();
    
    // 初始化週邊
    MX_GPIO_Init();
    MX_USB_Device_Init();
    MX_I3C1_Init();
    MX_I2C1_Init();  // Fallback
    
    printf("USB-I3C Bridge Ready\r\n");
    
    while (1) {
        // 處理 USB HID 資料
        if (usbDataReady) {
            ProcessUSBCommand(usbRxBuffer);
        }
        
        // 可選: 輪詢 I3C 裝置狀態
        // CheckI3CDevices();
    }
}

// 處理 USB 命令
void ProcessUSBCommand(uint8_t *cmd) {
    switch (cmd[0]) {
        case CMD_I3C_WRITE:
            I3C_WriteReg(cmd[1], cmd[2], &cmd[4], cmd[3]);
            break;
        case CMD_I3C_READ:
            I3C_ReadReg(cmd[1], cmd[2], responseBuffer, cmd[3]);
            USB_SendData(responseBuffer, cmd[3] + 4);
            break;
        case CMD_I3C_SCAN:
            // 掃描 I3C bus，回傳裝置清單
            break;
    }
}
```

---

## 6. 編譯與燒錄

### 6.1 使用 PlatformIO

```bash
# platformio.ini
[env:STM32H563ZIT6]
platform = ststm32
board = STM32H563ZIT6
framework = stm32cube
upload_protocol = stlink

[env:genericSTM32H563ZIT6]
platform = ststm32
board = genericSTM32H563ZIT6
framework = stm32cube
```

### 6.2 使用 STM32CubeIDE (CLI)

```bash
# 假設已安裝 STM32CubeIDE
# 編譯
"C:\ST\STM32CubeIDE\STM32CubeIDE.exe" -nosplash -application org.eclipse.cdt.managedbuilder.core.headlessbuild -project USB-I3C-Bridge

# 燒錄
"C:\ST\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe" -c port=SWD -w build\USB-I3C-Bridge.hex -rst
```

### 6.3 使用 OpenOCD + ARM GCC

```bash
# 編譯
make -j4

# 燒錄
openocd -f interface/stlink.cfg -f target/stm32h5x.cfg -c "program build/project.elf reset exit"
```

---

## 7. 參考資源

### 官方範例
| 資源 | 連結 |
|------|------|
| STM32CubeH5 | [GitHub](https://github.com/STMicroelectronics/STM32CubeH5) |
| I3C 範例 | `Projects/NUCLEO-H563ZI/Examples/I3C/` |
| USB HID 範例 | `Projects/NUCLEO-H563ZI/Examples/USB_Device/` |
| USB Composite H503 | [GitHub](https://github.com/stm32-hotspot/USBD_Composite_STM32H503) |

### 開源工具
| 工具 | 連結 |
|------|------|
| PlatformIO | [官網](https://platformio.org/) |
| ARM GCC | [ARM Developer](https://developer.arm.com/downloads/-/gnu-rm) |
| ST-Link tools | [GitHub](https://github.com/stlink-org/stlink) |
| OpenOCD | [官網](https://openocd.org/) |

### 文件
| 文件 | 連結 |
|------|------|
| AN5879 (I3C) | [PDF](https://www.st.com/resource/en/application_note/an5879-introduction-to-i3c-for-stm32h5-series-mcu-stmicroelectronics.pdf) |
| RM0481 (STM32H563 Reference Manual) | [PDF](https://www.st.com/resource/en/reference_manual/rm0481-stm32h563h573-and-stm32h562-armbased-32bit-mcus-stmicroelectronics.pdf) |
| UM1718 (CubeMX User Guide) | [PDF](https://www.st.com/resource/en/user_manual/um1718-stm32cubemx-software- setup-for-stm32-mcus-and-mpus-stmicroelectronics.pdf) |

---

## 8. 待完成工作

- [ ] 購買 NUCLEO-H563ZI 開發板
- [ ] 安裝 PlatformIO 或 GCC toolchain
- [ ] 下載 STM32CubeH5 FW Package
- [ ] 測試編譯 USB HID 範例
- [ ] 測試編譯 I3C 範例
- [ ] 實現 USB HID 通訊框架
- [ ] 實現 I3C 通訊框架
- [ ] 整合資料轉發邏輯
- [ ] 測試 USB-I3C Bridge 功能

---

## 9. 檔案位置

```
D:\Project\KM-collect\Hardware\I3C-USB-Bridge\
├── 04-STM32H563ZI-Development.md      # 開發資源總覽
├── 05-AN5879-I3C-STM32H5-Tutorial.md  # I3C 實作教學
├── 06-USB-I3C-Bridge-Firmware.md     # 本文件 - 韌體開發指南
```
