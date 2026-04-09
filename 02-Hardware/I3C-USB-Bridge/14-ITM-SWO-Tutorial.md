# ITM/SWO 除錯追蹤 - 掃盲教學

> 建立日期: 2026-03-27
> 適用: STM32 / Cortex-M 系列

---

## 🎯 目標讀者

- 第一次接觸嵌入式追蹤除錯
- 已經會用 `printf` 但不知 ITM/SWO 是什麼
- 想更有效率地除錯

---

## 🔰 基礎概念

### 什麼是 ITM?

**ITM (Instrumentation Trace Macrocell)** 是 ARM Cortex-M 內建的硬體模組，讓你可以：
- 輸出除錯訊息
- 計時效能分析
- 追蹤程式執行流程

### 什麼是 SWO?

**SWO (Serial Wire Output)** 是一條硬體訊號線，用於傳輸 ITM 資料

### 什麼是 SWV?

**SWV (Serial Wire Viewer)** 是軟體工具，負責接收並顯示 SWO 傳來的資料

### 三者關係

```
┌─────────────┐    SWO     ┌─────────────┐
│   ITM       │ ────────→  │   SWV      │
│ (MCU 內)    │   (PB3)    │ (IDE/工具)  │
└─────────────┘            └─────────────┘
```

---

## 🔌 硬體架構

### SWO 接腳

| MCU 系列 | SWO 接腳 | 備註 |
|----------|----------|------|
| STM32F1xx | PB3 | 預設 |
| STM32F4xx | PB3 | |
| STM32H5xx | PB3 | |
| STM32H7xx | PB3 或 PA13 | 依型號 |

### 接線圖

```
STM32 MCU              ST-Link Debugger
┌─────────────┐        ┌─────────────┐
│             │        │             │
│   PB3  ─────┼────────┼→ SWO        │
│   SWDIO ───┼────────┼→ SWDIO      │
│   SWCLK ───┼────────┼→ SWCLK      │
│   GND  ────┼────────┼→ GND        │
│             │        │             │
└─────────────┘        └─────────────┘
```

> NUCLEO 開發板已內建 SWO 接線，只需確認配置

---

## 🆚 ITM vs printf

| 特性 | printf (USART) | ITM/SWO |
|------|----------------|---------|
| **速度** | 慢 (115200 baud) | 快 (MHz 等級) |
| **即時性** | 需中斷 | 硬體輸出 |
| **資源** | 佔用 UART | 零額外資源 |
| **中斷影響** | 可能漏字 | 不會漏 |
| **同步/非同步** | 非同步 | 可同步 |

### 簡單來說

**printf** = 慢、會漏資料、需要設定
**ITM** = 快、不會漏、幾乎零開銷

---

## 💻 軟體設定 (STM32CubeIDE)

### Step 1: 啟用追蹤功能

在 `.ioc` 檔案中：

```
System Core → SYS → Trace Mode → Serial Wire (Trace Asynchronous)
```

### Step 2: 設定時脈

```
System Core → SYS → Trace Clock Sync → 設定為實際 MCU 時脈
```

### Step 3: 啟用 ITM 端點

在程式碼中啟用 ITM Port 0：

```c
// 啟用 ITM
ITM->TER |= (1 << 0);  // 啟用 Port 0
```

---

## 📝 程式碼實作

### 方法 1: 使用 ITM_SendChar (最簡單)

```c
#include "itm.h"

void debug_log(const char *msg) {
    while (*msg) {
        ITM_SendChar(*msg++);
    }
}

int main(void) {
    debug_log("Hello ITM!\n");
    
    uint32_t counter = 0;
    while (1) {
        counter++;
        if (counter % 1000000 == 0) {
            debug_log("Counter: ");
            // 可用 itm_print 輸出數值
        }
    }
}
```

### 方法 2: 重定向 printf 到 ITM

```c
// 最少干預版本
int _write(int file, char *ptr, int len) {
    for (int i = 0; i < len; i++) {
        ITM_SendChar(ptr[i]);
    }
    return len;
}

// 之後可直接使用 printf
printf("Value: %d\n", myVariable);
```

### 方法 3: 使用 DWT 計時

```c
#include "dwt.h"

void DWT_Init(void) {
    // 啟用 DWT (Data Watchpoint and Trace)
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

uint32_t DWT_GetCycles(void) {
    return DWT->CYCCNT;
}

// 使用
DWT_Init();
uint32_t start = DWT_GetCycles();
// ... 執行你要測量的程式碼 ...
uint32_t end = DWT_GetCycles();
uint32_t us = (end - start) / (SystemCoreClock / 1000000);
printf("Execution time: %lu us\n", us);
```

---

## 🖥️ CubeIDE 查看輸出

### 1. 開啟 SWV Console

```
Window → Show View → SWV → SWV ITM Data Console
```

### 2. 設定

- Trace Clock: 設定為你的 MCU 時脈 (如 250MHz)
- Enable ITM Port 0: 勾選

### 3. 啟動追蹤

點擊紅色 "Start Trace" 按鈕

---

## 🔧 常見問題排查

### Q: 沒有輸出？

| 檢查項目 | 解決方式 |
|----------|----------|
| PB3 接腳被佔用 | 確認 GPIO mode 為 AF0 |
| SWV 未開啟 | 確認 Trace Mode 已設定 |
| 時脈錯誤 | 確認 trace clock 正確 |
| Debug probe 不支援 | 確認 ST-Link 為 V2+ |

### Q: 顯示亂碼？

- 確認時脈設定正確
- 確認波特率 (baudrate) 匹配
- 嘗試較低速 (如 2MHz)

### Q: printf 不工作？

- 確認有 `#include <stdio.h>`
- 確認已實作 `_write()` 函數
- 確認 ITM port 已啟用

---

## 📊 實際應用場景

### 1. 效能分析

```c
// 測量函數執行時間
uint32_t start = DWT_GetCycles();
my_function();
uint32_t end = DWT_GetCycles();
printf("Function took %lu cycles\n", end - start);
```

### 2. 中斷追蹤

```c
void TIM2_IRQHandler(void) {
    ITM_SendChar('I');  // 標記中斷進入
    // ... 中斷處理 ...
    ITM_SendChar('O');  // 標記中斷離開
}
```

### 3. 狀態機追蹤

```c
enum State { IDLE, RUNNING, ERROR };
enum State currentState = IDLE;

void updateState(enum State newState) {
    currentState = newState;
    ITM_SendChar('0' + newState);
}
```

---

## 🔧 其他工具支援

| 工具 | 支援情況 |
|------|----------|
| **STM32CubeIDE** | ✅ SWV Console |
| **Keil MDK** | ✅ ITM Viewer |
| **IAR EWARM** | ✅ ITM Terminal |
| **PlatformIO** | ✅ SWO 支援 |
| **OpenOCD** | ✅ ITM 追蹤 |
| **J-Link** | ✅ J-Scope |

### PlatformIO 設定範例

```ini
[env:nucleo_h563zi]
platform = ststm32
board = nucleo_h563zi
framework = stm32cube

# 啟用 ITM
debug_extra_cmds =
    monitor swo create
    monitor itm port 0 on
```

---

## 📋 快速總結

| 項目 | 說明 |
|------|------|
| **ITM** | MCU 內建追蹤硬體 |
| **SWO** | 輸出接腳 (PB3) |
| **SWV** | IDE 端的檢視工具 |
| **優點** | 快、零開銷、即時 |
| **必備** | ST-Link V2+、CubeIDE |

---

## 📎 參考資源

- [ST Product Training: STM32 Debug](https://www.st.com/resource/en/product_training/STM32MP1-System-Debug_DBG.pdf)
- [ARM ITM Reference](https://developer.arm.com/documentation/den0013/latest)
- [PlatformIO ITM/SWO Guide](https://www.quantulum.co.uk/blog/platformio-arm-itm-swo-serial-wire-viewer/)
