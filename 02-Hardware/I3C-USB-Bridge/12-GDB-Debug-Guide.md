# GDB 除錯指南 - STM32H563ZI

> 建立日期: 2026-03-27
> 適用晶片: STM32H563ZI / Cortex-M33

---

## 什麼是 GDB?

**GDB (GNU Debugger)** 是跨平台的原始碼除錯工具，可用于：
- 逐步執行程式
- 設定斷點
- 檢視變數與記憶體
- 分析崩潰原因

---

## 基本指令速查

### 連線與執行

| 指令 | 說明 |
|------|------|
| `target remote host:port` | 連線到 debug server (OpenOCD) |
| `load` | 載入程式到 MCU |
| `run` | 開始執行 |
| `continue` (`c`) | 繼續執行 |
| `quit` (`q`) | 離開 GDB |

### 斷點

| 指令 | 說明 |
|------|------|
| `break main` | 在 main 函數設斷點 |
| `break file.c:50` | 在檔案第 50 行設斷點 |
| `break myFunction` | 在函數設斷點 |
| `delete <n>` | 刪除第 n 個斷點 |
| `info breakpoints` | 列出所有斷點 |

### 逐步執行

| 指令 | 說明 |
|------|------|
| `step` (`s`) | 逐步執行 (進入函數) |
| `next` (`n`) | 逐步執行 (跳過函數) |
| `finish` | 執行到函數結束 |
| `until <location>` | 執行到指定位置 |

### 檢視資料

| 指令 | 說明 |
|------|------|
| `print <var>` | 顯示變數值 |
| `print /x <var>` | 以 16 進位顯示 |
| `display <var>` | 每次暫停時自動顯示 |
| `info registers` | 顯示所有暫存器 |
| `info locals` | 顯示區域變數 |

### 記憶體檢視

| 指令 | 說明 |
|------|------|
| `x/32xb 0x20000000` | 讀取 32 bytes (byte format) |
| `x/16xw 0x20000000` | 讀取 16 words (hex format) |
| `x/s 0x20000000` | 以字串顯示 |

---

## GDB + OpenOCD 實戰

### 連線流程

**終端 1 - 啟動 OpenOCD:**
```bash
openocd -f interface/stlink-dap.cfg -f target/stm32h5x.cfg
```
輸出範例:
```
OpenOCD r0.12-00001-ga7d3f3d9
Info : Listening on port 3333 for gdb connections
```

**終端 2 - 啟動 GDB:**
```bash
arm-none-eabi-gdb firmware.elf
(gdb) target remote localhost:3333
(gdb) load
```

---

## 常見除錯情境

### 情境 1: HardFault 分析

```gdb
# 發生 HardFault 時
(gdb) print $pc           # 查看錯誤發生位置
(gdb) print $xPSR         # 查看異常類型
(gdb) backtrace           # 查看呼叫堆疊
(gdb) info registers      # 查看所有暫存器
```

**常見錯誤碼:**
- `0x00000001` - Reset
- `0x00000002` - NMI
- `0x00000003` - HardFault

### 情境 2: 記憶體監控 (Watchpoint)

```gdb
# 監控特定記憶體位置被寫入
(gdb) watch *(uint32_t*)0x20001234
(gdb) continue

# 監控特定位置被讀取
(gdb) rwatch *(uint32_t*)0x20001234
```

### 情境 3: 中斷除錯

```gdb
# 在特定中斷設斷點
(gdb) break HardFault_Handler
(gdb) break USART1_IRQHandler

# 查看中斷發生次數
(gdb) info breakpoints
```

### 情境 4: USB 除錯

```gdb
# 檢視 USB 暫存器
(gdb) print *(USB_TypeDef*)0x40040000

# 檢視Endpoint緩衝區
(gdb) x/64xb 0x40040000 + 0x20

# 在 USB 斷點設限
(gdb) break USB_IRQHandler
```

### 情境 5: I3C 通訊除錯

```gdb
# 檢視 I3C 暫存器
(gdb) print *(I3C_TypeDef*)0x4000AC00

# 監控資料緩衝區
(gdb) x/32xb 0x20001000
```

---

## GDB 腳本自動化

### 範例腳本 (debug.gdb)

```gdb
# 連線並載入
target remote localhost:3333
load

# 設定斷點
break main
break HardFault_Handler

# 開始執行
continue

# 自動化腳本可以設定條件
```

### 使用腳本

```bash
arm-none-eabi-gdb -x debug.gdb firmware.elf
```

---

## TUI 視覺化介面

```bash
# 啟動時自動開啟 TUI
arm-none-eui-gdb -tui firmware.elf

# GDB 內指令
(gdb) layout src     # 原始碼視圖
(gdb) layout reg     # 暫存器視圖
(gdb) layout split   # 分割視圖
(gdb) layout asm     # 組合語言視圖
(gdb) Ctrl+L         # 刷新畫面
```

---

## VS Code 整合

### launch.json 範例

```json
{
  "type": "cortex-debug",
  "request": "launch",
  "name": "Debug STM32H563ZI",
  "executable": "${workspaceFolder}/firmware.elf",
  "serverpath": "openocd",
  "configFiles": [
    "interface/stlink-dap.cfg",
    "target/stm32h5x.cfg"
  ],
  "searchDir": [],
  "runToEntryPoint": "main",
  "svdFile": "STM32H563.svd"
}
```

---

## 疑難排解

| 問題 | 解決方式 |
|------|----------|
| 無法連線 | 確認 STLINK 驅動正確安裝 |
| 讀取失敗 | 檢查 SWD 線路接線 |
| 卡住 | 按 `Ctrl+C` 中斷 |
| 斷點无效 | 確認編譯時有加 `-g` 參數 |

---

## 相關工具

| 工具 | 用途 |
|------|------|
| OpenOCD | Debug server |
| STLINK | Debug probe |
| STM32CubeCLT | 內含 GDB |
| arm-none-eabi-gcc | 編譯器 |

---

## 參考資源

- [GDB Manual](https://sourceware.org/gdb/documentation/)
- [OpenOCD STM32H5 設定](https://community.st.com/t5/stm32-mcus-products/nucleo-stm32h5-debugging-openocd/td-p/618225)
