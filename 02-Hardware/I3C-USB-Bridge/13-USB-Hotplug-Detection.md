# USB 熱插拔繼電器控制方案

> 建立日期: 2026-03-27
> 目的: 使用繼電器硬體模擬 USB 熱插拔

---

## ⚠️ 挑戰

USB 熱插拔涉及 **4條線**:
| 線 | 說明 | 速度要求 |
|---|---|---|
| VBUS | 5V 電源 | DC |
| D+ | 資料+ | 480 Mbps |
| D- | 資料- | 480 Mbps |
| GND | 地線 | DC |

**難點:** D+/D- 是高速訊號，機械繼電器無法快速切換

---

## 🔌 方案一: 只控制 VBUS (推薦)

最簡單方案 - 只用繼電器控制電源

```
┌─────────────────────────────────────────────┐
│                  USB 主機                      │
└─────────────────┬───────────────────────────┘
                  │
                  │ VBUS (5V)
                  │
            ┌─────▼─────┐
            │   RELAY   │ (5V 線圈)
            │  SPST-NO  │
            └─────┬─────┘
                  │ 
                  ▼ (NC 常閉 → NO 常開)
            ┌─────┬─────┐
            │     │     │
          VBUS   D+    D-
            │     │     │
            └─────┴─────┘
                  │
                  ▼
            ┌─────────────────┐
            │   USB 設備      │
            │  (STM32H563ZI)  │
            └─────────────────┘
```

### 繼電器選型

| 型號 | 特性 | 用途 |
|------|------|------|
| **G6K-2F-Y** | 5V, 2A, DPDT | 訊號切換 |
| **HF115F** | 5V, 10A, SPDT | 電源切換 |
| **SMD 繼電器** | 體積小 | PCB 整合 |

### 電路圖

```
         VBUS (5V) from Host
              │
         ┌────┴────┐
         │  Relay  │ (如 HF115F)
         │  SPST   │
         │         │
         └────┬────┘
              │
              ▼ 輸出 to STM32 USB VBUS
              │
         ┌────┴────┐
         │  4.7kΩ  │ ──┐
         │  10kΩ   │   ├── 分壓給 PA9
         └────┬────┘ ──┘
              │
             GND
```

### 控制腳本

```python
# usb_relay_control.py
import serial
import time

class USBHotplugRelay:
    def __init__(self, com_port, relay_pin):
        self.ser = serial.Serial(com_port, 9600)
        self.relay_pin = relay_pin
    
    def connect(self):
        """模擬 USB 插入"""
        self.ser.write(b"RELAY ON\n")
        print("USB Connected")
    
    def disconnect(self):
        """模擬 USB 拔出"""
        self.ser.write(b"RELAY OFF\n")
        print("USB Disconnected")

# 使用範例
relay = USBHotplugRelay("COM3", 2)
relay.connect()    # 繼電器吸合，VBUS 接通
time.sleep(0.5)
relay.disconnect() # 繼電器釋放，VBUS 切斷
```

---

## 🔌 方案二: 完全隔離 (含 Data 切換)

需要切換資料線時，使用 **類比開關 IC**

### 元件選型

| 元件 | 型號 | 說明 |
|------|------|------|
| **類比開關** | 74HC4066 或 TS5A23159 | 切換 D+/D- |
| **電源繼電器** | G5LA-1 | 切換 VBUS |
| **光耦合器** | PC817 | 控制訊號隔離 |

### 電路圖

```
┌─────────────────────────────────────────────────────┐
│                      主機                            │
│   VBUS    D+    D-    GND                          │
└─────┬─────┬─────┬─────┬────────────────────────────┘
      │     │     │     │
 ┌────┴┐ ┌──┴──┐ ┌──┴──┐ │
 │Relay│ │HC4066│ │HC4066│ │ (受控於 MCU)
 │ VBUS│ │ D+  │ │ D-  │ │
 └────┬┘ └──┬──┘ └──┬──┘ │
      │      │       │    │
      └──────┴───────┴────┘
                │
               to STM32
              USB pins
```

### 控制時序

```c
// 模擬插入
void simulate_usb_insert(void) {
    // 1. 先接通 VBUS
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);  // 繼電器吸合
    
    // 2. 等待 VBUS 穩定 (10ms)
    HAL_Delay(10);
    
    // 3. 接通資料線
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);  // HC4066 Enable
    
    // 4. 等待資料線穩定 (1ms)
    HAL_Delay(1);
    
    // 5. 初始化 USB
    MX_USB_DEVICE_Init();
}

// 模擬拔出
void simulate_usb_remove(void) {
    // 1. 先斷開資料線
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);  // HC4066 Disable
    
    // 2. 等待資料線穩定 (1ms)
    HAL_Delay(1);
    
    // 3. 斷開 VBUS
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);  // 繼電器釋放
    
    // 4. 清理 USB 狀態
    USBD_DeInit(&hUsbDeviceFS);
}
```

---

## 🔌 方案三: USB 切換器 IC (最佳方案)

使用專用 USB 切換 IC

### 推薦 IC

| 型號 | 廠商 | 頻寬 | 特性 |
|------|------|------|------|
| **USB2512** | Microchip | 480 Mbps | 2-Port Hub |
| **USB2532** | Microchip | 480 Mbps | 4-Port Hub |
| **PI5CUSB3120** | Diodes | 11.5 GHz | USB 3.1 切換 |
| **NTTS2R06** | ON Semiconductor | 480 Mbps | 低阻抗 |

### 電路圖 (使用 NTTS2R06)

```
            ┌──────────────┐
  USB_HOST ─┤ A1    D+    B1 ├─ to STM32 D+
            │               │
            │ A2    D-    B2 ├─ to STM32 D-
            │               │
  ENABLE ───┤ EN           │
            └──────────────┘

            ┌──────────────┐
  VBUS ─────┤ Relay       ├─ to STM32 VBUS
            │ (G5LA-1)    │
  CONTROL ──┤ Coil        │
            └──────────────┘
```

---

## 📋 完整線路清單

### 方案一 (只控制 VBUS) - 最簡單

| 元件 | 型號/規格 | 數量 | 價格參考 |
|------|-----------|------|----------|
| 繼電器 | HF115F-5V | 1 | NT$30 |
| 二極管 | 1N4148 | 1 | NT$1 |
| 電阻 | 4.7kΩ + 10kΩ | 2 | NT$2 |
| 電容 | 100nF | 1 | NT$1 |
| **總計** | | | **~NT$35** |

### 方案二 (完全隔離)

| 元件 | 型號/規格 | 數量 | 價格參考 |
|------|-----------|------|----------|
| 繼電器 | G5LA-1 5V | 1 | NT$15 |
| 類比開關 | 74HC4066 | 1 | NT$10 |
| 光耦合器 | PC817 | 1 | NT$5 |
| 二極管 | 1N4148 | 1 | NT$1 |
| 電阻 | 4.7kΩ + 10kΩ | 2 | NT$2 |
| **總計** | | | **~NT$35** |

---

## ⚡ 安全注意事項

1. **切勿在 USB 傳輸時切斷 VBUS** - 可能損壞裝置
2. **先切資料線，再切 VBUS** - 正確順序
3. **增加去抖動電容** - 100uF 電容在 VBUS
4. **ESD 保護** - USBLC6-2SC6

### 錯誤順序 (會損壞!)
```
X 拔出時: 先切 VBUS → 再切資料線
✓ 拔出時: 先切資料線 → 再切 VBUS

X 插入時: 先接資料線 → 再接 VBUS  
✓ 插入時: 先接 VBUS → 等待穩定 → 再接資料線
```

---

## 🔧 實作建議

### 麵包板測試
1. 先用 Arduino 控制繼電器測試
2. 確認 VBUS 訊號正確
3. 再進行資料線測試

### PCB 設計
1. 使用 USB 專用連接器 (Micro-B 或 Type-C)
2. 保持 D+/D- 對稱走線 (90Ω 差分阻抗)
3. VBUS 走線要粗 (1A 以上)

---

## 📎 參考資源

- 74HC4066 Datasheet
- AN4879 - USB Hardware Guidelines
- USB 2.0 Specification
