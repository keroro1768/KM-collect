# M487JIDAE 燒錄工具與環境

## 📁 目錄結構

```
D:\AiWorkSpace\KM\M487\
├── M480BSP/                     # Nuvoton M480 BSP (含 M487 驅動與範例)
│   ├── Library/StdDriver/src/   # 標準驅動程式
│   ├── SampleCode/              # 範例程式
│   │   ├── NuMaker-ETM-M487/
│   │   ├── NuMaker-M487KMCAN/
│   │   └── NuMaker-PFM-M487/
│   └── Document/
├── flash_backup/
│   ├── m487_flash.bin           # 完整 Flash 備份 (512KB)
│   └── m487_flash_backup.bin    # 備份副本
├── tools/
│   ├── read_m487_flash.tcl      # Flash 讀取腳本
│   └── write_m487_flash.tcl     # Flash 燒錄腳本
├── 00_INDEX.md                  # 本文件
├── 01_SPEC.md                   # M487 晶片規格
├── 02_TOOLS.md                  # 工具安裝與設定
├── 03_FLASH_READ_WRITE.md       # Flash 讀寫流程
├── 04_KEIL_SETUP.md             # Keil MDK 燒錄指南
├── 05_OPENOCD_REF.md            # OpenOCD 常用指令
└── datasheet_download.md        # Datasheet 下載說明
```

---

## 📥 M480BSP (作為 Git Submodule)

> 注意：`M480BSP/` 為獨立的 Git repo，請用以下方式加入：
> ```bash
> cd Hardware/M487
> git submodule add https://github.com/OpenNuvoton/M480BSP.git M480BSP
> ```



| 項目 | 內容 |
|------|------|
| **位置** | `D:\AiWorkSpace\KM\M487\M480BSP\` |
| **Clone URL** | https://github.com/OpenNuvoton/M480BSP.git |

### 驅動程式 (Library/StdDriver/src/)

| 檔案 | 功能 |
|------|------|
| `gpio.c` | GPIO 控制 |
| `uart.c` | UART 序列通訊 |
| `spi.c` | SPI 介面 |
| `i2c.c` | I2C 介面 |
| `timer.c` | 定時器 |
| `pwm.c` / `bpwm.c` / `epwm.c` | PWM 控制 |
| `adc.c` / `eadc.c` | ADC 轉換 |
| `dac.c` | DAC 輸出 |
| `usb.c` / `usbd.c` / `hsusbd.c` | USB 裝置 |
| `can.c` | CAN 匯流排 |
| `crypto.c` | 加密引擎 |
| `fmc.c` | Flash 控制 |
| `clk.c` | 時脈管理 |
| `rtc.c` | 即時時鐘 |
| `spi.c` / `qspi.c` / `spim.c` | Flash/SPI |
| `sdh.c` | SD Host |
| `pdma.c` | DMA 控制 |
| `ebi.c` | External Bus Interface |
| `ecap.c` / `ccap.c` | Capture |

### 範例程式 (SampleCode/)

- `NuMaker-ETM-M487/` — ETM 追蹤範例
- `NuMaker-M487KMCAN/` — CAN 範例
- `NuMaker-PFM-M487/` — PFM 控制範例

---

## 🔧 工具下載連結

### OpenOCD
- **位置**: `C:\Users\rinry\Tool\OpenOCD-Nuvoton\`
- **版本**: 0.10.022.0-dev-00493

### Keil MDK
- 官網: https://www.keil.arm.com/demo/eval/arm.htm
- Nuvoton License: https://www.nuvoton.com/tool-and-software/ide-and-compiler/keil-mdk-nuvoton-edition/application-form/

### NuEclipse
- Windows: https://www.nuvoton.com/resource-download.jsp?tp_GUID=SW132022111608043510

---

## ⚙️ 環境狀態 (2026-03-26)

| 項目 | 狀態 | 備註 |
|------|------|------|
| OpenOCD | ✅ 已設定 | Nu-Link 可被識別 |
| M480BSP | ✅ 已下載 | `D:\AiWorkSpace\KM\M487\M480BSP\` |
| Keil MDK | ⚠️ 需設定 | 需安裝 M4 DFP |
| Nu-Link 驅動 | ✅ 已安裝 | Interface 1: WinUSB |
| Flash 備份 | ✅ 已完成 | `m487_flash.bin` 512KB, MD5: F8618AA1 |
