# USB MSC 開發資源

> 建立日期: 2026-03-27
> 適用: STM32H5 系列

---

## 📋 概述

USB MSC (Mass Storage Class) 使 STM32 作為 USB 隨身碟呈現，可讓 PC 直接存取儲存媒體。

---

## 🏁 官方範例

### STM32CubeH5 USBX 範例

**GitHub:** https://github.com/STMicroelectronics/stm32-usbx-examples

```
stm32-usbx-examples/
├── README.md
└── Projects/
    └── NUCLEO-H563ZI/
        └── Applications/
            └── USBX/
                └── Ux_Device_MSC_xxx/  (依版本而定)
```

### ST Community 教學

| 文章 | 連結 |
|------|------|
| USB Mass Storage Device (U5G9J-DK1) | [Community](https://community.st.com/t5/stm32-mcus/how-to-implement-usb-mass-storage-device-standalone-on-stm32/ta-p/841325) |
| USB Device MSC (經典版) | [Community](https://community.st.com/t5/stm32-mcus/how-to-use-stmicroelectronics-classic-usb-device-middleware-with/ta-p/599274) |

---

## 🔧 第三方開源專案

### 1. shishir-dey/stm32-usb-msc

**GitHub:** https://github.com/shishir-dey/stm32-usb-msc

**技術堆疊:**
- STM32 + USB_OTG
- USB Mass Storage Class
- FatFS (檔案系統)

**功能:**
- 檔案讀寫
- SD Card 支援
- RAM Disk 支援

### 2. STM32GENERIC

**網站:** https://danieleff.github.io/STM32GENERIC/usb_msc/

**特色:**
- Arduino 相容
- RamBlockDevice (RAM 磁碟)
- 適合測試

---

## 📚 教學資源

### ControllersTech USB MSC 教學

**網站:** https://controllerstech.com/stm32-usb-msc/

**內容:**
1. CubeMX 設定
2. USB MSC 初始化
3. RAM 作為儲存
4. SD Card 整合

### ARM MDK USB Component

**網站:** https://arm-software.github.io/MDK-Middleware/main/USB/usbd_example_msc.html

**範例檔案:**
- MassStorage.c
- MemoryDiskImage.c
- USBD_User_MSC_0.c

---

## 💻 程式碼結構

### 核心 API

```c
// USB MSC 初始化
USBD_StatusTypeDef USBD_Init(USBD_HandleTypeDef *pdev, 
                             USBD_DescriptorsTypeDef *pdesc, 
                             uint8_t id);

USBD_StatusTypeDef USBD_RegisterClass(USBD_HandleTypeDef *pdev, 
                                       USBD_ClassTypeDef *pclass);

USBD_StatusTypeDef USBD_MSC_RegisterStorage(USBD_HandleTypeDef *pdev, 
                                             USBD_StorageTypeDef *fops);
```

### 儲存媒體 Callbacks

```c
// 需要實作的儲存 API
typedef struct {
    int8_t (*Init)(uint8_t lun);
    int8_t (*Read)(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
    int8_t (*Write)(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
    int8_t (*GetCapacity)(uint8_t lun, uint32_t *blk_nbr, uint16_t *blk_size);
    int8_t (*IsReady)(uint8_t lun);
    int8_t (*IsWriteProtected)(uint8_t lun);
    int8_t (*GetMaxLun)(void);
    int8_t *pInquiry;
} USBD_StorageTypeDef;
```

### 完整範例 (RAM Disk)

```c
// main.c
#include "usbd_core.h"
#include "usbd_msc.h"

USBD_HandleTypeDef hUsbDeviceFS;
USBD_StorageTypeDef USBD_DISK_fops;

// RAM Disk 配置 (測試用)
int8_t MSC_Init(uint8_t lun) {
    return 0;
}

int8_t MSC_Read(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len) {
    // 從 RAM 讀取區塊
    memcpy(buf, ram_disk + blk_addr * blk_len, blk_len * 512);
    return 0;
}

int8_t MSC_Write(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len) {
    // 寫入 RAM 區塊
    memcpy(ram_disk + blk_addr * blk_len, buf, blk_len * 512);
    return 0;
}

int8_t MSC_GetCapacity(uint8_t lun, uint32_t *blk_nbr, uint16_t *blk_size) {
    *blk_nbr = RAM_DISK_SIZE / 512;  // 區塊數
    *blk_size = 512;                   // 區塊大小
    return 0;
}

int8_t MSC_GetMaxLun(void) {
    return 0;
}

int8_t *MSC_InquiryData[1] = {
    (int8_t *)"STM32  ",  // Vendor
    "RAM Disk  ",         // Product
    "1.00  "              // Version
};

void USB_Init(void) {
    // 註冊儲存介面
    USBD_DISK_fops.Init = MSC_Init;
    USBD_DISK_fops.Read = MSC_Read;
    USBD_DISK_fops.Write = MSC_Write;
    USBD_DISK_fops.GetCapacity = MSC_GetCapacity;
    USBD_DISK_fops.GetMaxLun = MSC_GetMaxLun;
    
    // 初始化 USB
    USBD_Init(&hUsbDeviceFS, &HS_Desc, DEVICE_FS);
    USBD_RegisterClass(&hUsbDeviceFS, &USBD_MSC_CLASS);
    USBD_MSC_RegisterStorage(&hUsbDeviceFS, &USBD_DISK_fops);
    USBD_Start(&hUsbDeviceFS);
}
```

---

## 🗄️ 儲存媒體選項

| 媒體 | 複雜度 | 速度 | 適合場景 |
|------|--------|------|----------|
| **RAM** | ⭐ | 快 | 測試、暫存資料 |
| **Internal Flash** | ⭐⭐ | 快 | 設定儲存 |
| **SD Card** | ⭐⭐⭐ | 中 | 資料紀錄、日誌 |
| **eMMC** | ⭐⭐⭐⭐ | 快 | 大容量儲存 |

### RAM Disk 配置範例

```c
// 定義 RAM Disk 大小 (64KB)
#define RAM_DISK_SIZE (64 * 1024)
static uint8_t ram_disk[RAM_DISK_SIZE] __attribute__((aligned(4)));
```

### SD Card + FatFS

需要額外配置:
- FatFS middleware
- SD Card HAL driver
- SPI 或 SDIO 介面

---

## ⚙️ CubeMX 設定

### Step 1: USB 設定

```
USB_OTG_FS
├── Mode: Device_Only
├── Speed: Full_Speed (12Mbps)
└── DMA: Disabled
```

### Step 2: USB Device 設定

```
USB_DEVICE
├── Class: Mass Storage Class
├── MSC Media: RAM / SD Card / Flash
└── VBUS Sensing: Disabled
```

### Step 3: 時脈設定

```
USB Clock: 48MHz (必須準確)
```

---

## 🔍 常見問題

### Q: PC 不認 USB MSC 裝置？

| 檢查 | 說明 |
|------|------|
| USB 時脈 | 必須為 48MHz |
| VBUS 檢測 | 確認 VBUS 連接 |
| 描述符 | 確認 MSC 描述符正確 |

### Q: 讀寫速度慢？

- 使用 DMA
- 增加緩衝區大小
- 考慮切換到 USB 2.0 High-Speed

### Q: 檔案系統問題？

- 格式化為 FAT32
- 確認區塊大小為 512 bytes
- 檢查 MSC GetCapacity 回傳值

---

## 📎 參考資源

- [STM32 USB MSC Tutorial - ControllersTech](https://controllerstech.com/stm32-usb-msc/)
- [shishir-dey/stm32-usb-msc](https://github.com/shishir-dey/stm32-usb-msc)
- [STM32GENERIC USB MSC](https://danieleff.github.io/STM32GENERIC/usb_msc/)
- [STM32 USBX Examples](https://github.com/STMicroelectronics/stm32-usbx-examples)
