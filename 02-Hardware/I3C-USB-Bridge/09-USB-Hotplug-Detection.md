# USB 熱插拔偵測電路設計

> 建立日期: 2026-03-26

---

## 1. 熱插拔偵測原理

USB 熱插拔偵測有兩種主要方式：

### 方式 A: VBUS 偵測 (推薦)

| 項目 | 說明 |
|------|------|
| **偵測目標** | USB VBUS (+5V) 是否存在 |
| **使用時機** | 需要知道 USB 是否連接/移除 |
| **優點** | 可在不重新插拔的情況下偵測 |

### 方式 B: D+/D- pull-up 偵測

| 項目 | 說明 |
|------|------|
| **偵測目標** | 主機端pull-up電阻變化 |
| **限制** | 需要依賴 USB 協定棧 |

---

## 2. STM32 VBUS 偵測電路

### 2.1 基本電路 (最簡單)

```
┌────────────────────────────────────────────────────────┐
│                    USB VBUS 偵測                       │
│                                                        │
│  USB VBUS ───┬───► 3.3V ( MCU供電時 )                  │
│  (+5V)      │                                           │
│             │                                           │
│         ┌───┴───┐                                      │
│         │  R1   │  10K ~ 22K Ω (分壓)                  │
│         └───┬───┘                                      │
│             │                                           │
│             ▼ PA9 (或任何 GPIO)                         │
│                                                        │
│   MCU ────────────────────────────────────────────      │
│                                                        │
│  當 USB 連接: VBUS = 5V → GPIO = 3.3V (HIGH)          │
│  當 USB 移除: VBUS = 0V → GPIO = 0V   (LOW)           │
└────────────────────────────────────────────────────────┘
```

### 2.2 完整電路 (含保護)

```
┌─────────────────────────────────────────────────────────┐
│                  USB VBUS 偵測 (含保護)                 │
│                                                         │
│   USB VBUS ────►│──── R1 (10KΩ) ────► PA9              │
│    (+5V)       │  │                                      │
│                 │  │                                      │
│              ┌──┴──┐                                     │
│              │ Zener│  3.6V (保護 MCU)                   │
│              │  diodes│ (可選)                           │
│              └──────┘                                     │
│                                                         │
│   電阻分壓計算:                                          │
│   VBUS = 5V                                              │
│   R1 = 10KΩ, R2 = 10KΩ → GPIO 電壓 = 2.5V             │
│   R1 = 15KΩ, R2 = 10KΩ → GPIO 電壓 = 2V               │
│                                                         │
│   建議: 15KΩ + 10KΩ = 3V (MCU 3.3V 供電時安全)        │
└─────────────────────────────────────────────────────────┘
```

---

## 3. STM32 硬體連接

### 3.1 引腳選擇

| STM32 系列 | 建議引腳 | 備註 |
|------------|----------|------|
| STM32H5 | PA9, PA10 | 原生 USB VBUS sense |
| STM32F4 | PA9 | 預設 VBUS sensing |
| STM32L4 | PA9, PC0 | 可配置 |

### 3.2 STM32CubeMX 配置

```
1. USB_OTG_FS → Mode: Device_Only
2. Parameter Settings:
   - VBUS Sensing: Enable (若使用 PA9)
   - 或者手動設定 GPIO

3. GPIO 配置:
   - PA9: Input (上拉/下拉依據電路)
```

---

## 4. 韌體實作

### 4.1 方法一: EXTI 中斷 (推薦)

```c
// main.c - 初始化
void MX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // 啟用 GPIO 時鐘
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // 配置 PA9 為輸入
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;  // 預設下拉
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // 啟用 EXTI 中斷
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}

// EXTI callback
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == GPIO_PIN_9) {
        if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_9) == GPIO_PIN_SET) {
            // USB 連接 → 初始化 USB
            printf("USB Connected!\n");
            MX_USB_Device_Init();
        } else {
            // USB 移除 → 停止 USB
            printf("USB Disconnected!\n");
            USB_Device_DeInit();
        }
    }
}
```

### 4.2 方法二: Polling 輪詢

```c
// main loop 中輪詢
while (1) {
    static uint8_t last_state = 0;
    uint8_t current_state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_9);

    if (current_state != last_state) {
        if (current_state == GPIO_PIN_SET) {
            MX_USB_Device_Init();  // USB 連接
        } else {
            USB_Device_DeInit();   // USB 移除
        }
        last_state = current_state;
    }

    HAL_Delay(100);  // 100ms 輪詢間隔
}
```

### 4.3 方法三: USB事件回調

```c
// usbd_conf.c 中
void HAL_PCDEx_VbusConfig(PCD_HandleTypeDef *hpcd, uint8_t state) {
    // state = 1: VBUS present
    // state = 0: VBUS absent
    if (state == 1) {
        // USB 已連接
    } else {
        // USB 已移除
    }
}
```

---

## 5. USB 熱插拔後的處理

### 5.1 連接時

```c
void USB_Connect(void) {
    // 1. 初始化 USB 週邊
    MX_USB_Device_Init();

    // 2. 啟用 USB 中斷
    HAL_NVIC_SetPriority(USB_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(USB_IRQn);

    // 3. 連接 USB D+ pull-up (若需要)
    // HAL_PCD_Start(hpcd);
}
```

### 5.2 移除時

```c
void USB_Disconnect(void) {
    // 1. 停止 USB
    HAL_PCD_Stop(hpcd);

    // 2. 清除 pending 中斷
    HAL_NVIC_ClearPendingIRQ(USB_IRQn);

    // 3. 停用 USB 時鐘 (可選，省電)
    // __HAL_RCC_USB_CLK_DISABLE();

    printf("USB device stopped\n");
}
```

---

## 6. 電路保護考量

### 6.1 問題: MCU 未供電但 USB 連接

| 問題 | 說明 |
|------|------|
| **現象** | USB VBUS 5V 進入未供電的 MCU GPIO |
| **風險** | 可能損壞 MCU |

### 6.2 解決方案

| 方案 | 電路 | 說明 |
|------|------|------|
| **電阻分壓** | 15K + 10K | 降至 3V 以內 |
| **Zener 保護** | 3.3V Zener | 箝位電壓 |
| **MOSFET 隔離** | P-MOSFET | 完全隔離 |

### 6.3 推薦電路 (最安全)

```
┌─────────────────────────────────────────────────────────┐
│           USB VBUS 偵測 (MCU保護電路)                    │
│                                                         │
│   USB VBUS ──── R1 ───┬───► GPIO (STM32)                │
│   (+5V)               │                                         │
│                        │                                         │
│              ┌────────┴────────┐                              │
│              │   BAT54C (Diode)│  肖特基二極管               │
│              │   或 similar    │  防止反向電流               │
│              └─────────────────┘                              │
│                                                         │
│   R1 = 10KΩ ~ 15KΩ                                       │
│   GPIO 已內建 ESD 保護 (通常 15KV)                        │
└─────────────────────────────────────────────────────────┘
```

---

## 7. NUCLEO-H563ZI 上的 VBUS

### 硬體配置

| 項目 | NUCLEO-H563ZI |
|------|---------------|
| VBUS pin | USB  connector pin 1 |
| 偵測GPIO | 可用 PA9 或自訂 |
| 原生支援 | STM32H5 有 VBUS sensing 專用功能 |

### 電路連接範例

```
NUCLEO-H563ZI:
├── USB Type-C connector
│   ├── VBUS → (可連接至 PA9 進行偵測)
│   ├── D+
│   └── D-
└── PA9 → 10KΩ → GND (若使用內建 VBUS sensing)
```

---

## 8. 總結

### 實現熱插拔所需元件

| 項目 | 說明 |
|------|------|
| **偵測電路** | 電阻分壓 (10K+10K 或 15K+10K) |
| **GPIO** | PA9 (推薦) 或其他可用 GPIO |
| **韌體** | EXTI 中斷 或 Polling |

### 關鍵代碼

1. GPIO 初始化為輸入
2. 配置 EXTI 中斷
3. 在 callback 中處理連接/移除事件
4. 呼叫 USB_Init() / USB_DeInit()

### 測試方法

1. 連接 USB → 觀察 GPIO 變為 HIGH
2. 移除 USB → 觀察 GPIO 變為 LOW
3. 主機端應看到 USB device enum/de-enum
