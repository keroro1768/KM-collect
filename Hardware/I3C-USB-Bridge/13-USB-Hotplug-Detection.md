# USB 熱插拔偵測 - 硬體電路與軟體控制

> 建立日期: 2026-03-27
> 適用: STM32H563ZI USB Device

---

## 📋 概述

USB 熱插拔偵測包含兩個層面：
1. **硬體層**: VBUS 電壓偵測與保護電路
2. **軟體層**: 中斷處理與狀態管理

---

## 🔌 硬體電路設計

### 1. VBUS 偵測電路 (推薦)

**用途**: 偵測 USB 插入/拔出事件

**電路圖:**
```
USB_VBUS ──┬── R1=4.7kΩ ──┬── PA9 (VBUS_SENSE)
           │              │
           └── R2=10kΩ ───┴── GND
```

**分壓計算:**
- VBUS = 5V (USB 標準)
- PA9 電壓 = 5V × (10k / (4.7k + 10k)) = 3.4V
- 安全邊界: 3.4V < 3.3V MCU IO max ✓

**STM32 連接對照:**

| 功能 | STM32H563ZI Pin | 備註 |
|------|-----------------|------|
| VBUS_SENSE | PA9 | USB VBUS 偵測 |
| USB_DM | PA11 | D- |
| USB_DP | PA12 | D+ |
| USB_PWR | PB1 | 電源控制 (可選) |

### 2. USB 電源保護電路

**用途**: 防止回流電流與過載

**方案 A: 簡單防護 (推薦)**
```
USB_VBUS ──┬── D1 ( Schottky ) ──┬── VDD_5V
           │                     │
           └── R100Ω ───────────┴── (MCU VBUS)
```

**方案 B: 獨立供電切換**
```
                        ┌── VDD_3V3 (MCU)
                        │
VBUS ── TPS2041 ───────┤
(IN)    (0.5A)          │
                        └── VDD_5V (USB Device)
```

**TPS2041 特性:**
| 參數 | 數值 |
|------|------|
| 輸出電流 | 0.5A |
| 限流保護 | 0.9A |
| Rds(on) | 80mΩ |
| 熱保護 | ✅ |

### 3. 防護元件清單

| 元件 | 型號 | 數量 | 備註 |
|------|------|------|------|
| 分壓電阻 | 4.7kΩ + 10kΩ | 2 | 1% 精度的 |
| 保護二極管 | BAT54S | 1 | 防反向 |
| 電容 | 100nF | 1 | VBUS 濾波 |
| USB 保護 IC | USBLC6 | 1 | USB 專用 ESD |
| 保險絲 | 500mA | 1 | 自恢復 PPTC |

---

## 🔧 STM32 軟體配置

### 1. VBUS 偵測中斷設定

**CubeMX 配置:**
```
Pin: PA9
Mode: GPIO_Input 或 VBUS_SENSE
Pull: Pull-Down
```

**中斷配置:**
```
EXTI Line: EXTI_Line9
Trigger: Rising/Falling Edge
```

### 2. USB 熱插拔中斷程式碼

```c
// usb_hotplug.c

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == GPIO_PIN_9) {
        // 讀取 VBUS 狀態
        if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_9) == GPIO_PIN_SET) {
            // USB 插入
            USB_Device.connected = true;
            printf("USB Connected\n");
        } else {
            // USB 拔出
            USB_Device.connected = false;
            printf("USB Disconnected\n");
            
            // 清理 USB 狀態
            USBD_Stop();
        }
    }
}
```

### 3. USB 初始化/去初始化

```c
// usb_hotplug.c

void USB_HotPlug_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // PA9 VBUS 偵測
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    // 初始狀態檢查
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_9) == GPIO_PIN_SET) {
        USB_Device.connected = true;
    }
}

void USB_Handle_Connection(void) {
    if (USB_Device.connected && !USB_Device.initialised) {
        // 初始化 USB
        MX_USB_DEVICE_Init();
        USB_Device.initialised = true;
    }
}

void USB_Handle_Disconnection(void) {
    if (!USB_Device.connected && USB_Device.initialised) {
        // 去初始化 USB
        USBD_DeInit(&hUsbDeviceFS);
        USB_Device.initialised = false;
    }
}
```

### 4. 主迴圈整合

```c
// main.c

int main(void) {
    // ... init ...
    
    USB_HotPlug_Init();
    
    while (1) {
        // 檢查 USB 連線狀態變化
        if (USB_Device.connected != USB_Device.last_state) {
            USB_Device.last_state = USB_Device.connected;
            
            if (USB_Device.connected) {
                USB_Handle_Connection();
            } else {
                USB_Handle_Disconnection();
            }
        }
        
        // 其他任務...
        HAL_Delay(100);
    }
}
```

---

## 🔒 安全機制

### 1. 防呆保護

| 保護項目 | 實現方式 |
|----------|----------|
| 去抖動 | 軟體延遲 100ms 確認狀態 |
| 多次偵測 | 連續 3 次狀態一致才生效 |
| 順序檢查 | 先確認 VBUS，再初始化 USB |

### 2. 狀態機

```
┌─────────┐   VBUS ON    ┌────────────┐
│ DISCONNECTED │───────────→│ CONNECTING │
└─────────┘               └────────────┘
    ↑                           │
    │                     USB OK │
    │    VBUS OFF         ┌──────▼──────┐
    └─────────────────────│ CONNECTED   │
                           └─────────────┘
```

### 3. 錯誤處理

```c
typedef enum {
    USB_STATE_DETACHED = 0,
    USB_STATE_ATTACHED,
    USB_STATE_POWERED,
    USB_STATE_DEFAULT,
    USB_STATE_ADDRESSED,
    USB_STATE_CONFIGURED,
    USB_STATE_SUSPENDED
} USB_StateTypeDef;

void USB_Error_Handler(USB_StateTypeDef state) {
    // 記錄錯誤
    Log_Error(state);
    
    // 嘗試恢復
    USB_DeInit();
    HAL_Delay(100);
    USB_Init();
}
```

---

## 📝 測試檢查清單

### 硬體測試
- [ ] VBUS 電壓正確 (5V ± 10%)
- [ ] PA9 電壓正確 (3.3V when connected)
- [ ] 電源保護電路運作正常
- [ ] ESD 保護正常

### 軟體測試
- [ ] 插入時正確偵測
- [ ] 拔出時正確偵測
- [ ] 多次插拔穩定性
- [ ] 中斷不會遺漏

### 相容性測試
- [ ] Windows
- [ ] macOS
- [ ] Linux
- [ ] 不同 USB 埠 (USB 2.0 / 3.0)

---

## 📎 參考文件

- AN4879 - USB Hardware and PCB Guidelines
- AN4879: https://www.st.com/resource/en/application_note/an4879
- TPS2041 Datasheet: https://www.ti.com/product/TPS2041
- STM32H563 Reference Manual
