# STM32H563ZI & NUCLEO-H563ZI 開發資源

> 建立日期: 2026-03-26
> 晶片: STM32H563ZIT6
> 開發板: NUCLEO-H563ZI

---

## 📄 主要文件

| 文件 | 連結 | 說明 |
|------|------|------|
| **AN5879** - I3C Introduction for STM32H5 | [PDF](https://www.st.com/resource/en/application_note/an5879-introduction-to-i3c-for-stm32h5-series-mcu-stmicroelectronics.pdf) | I3C 協定介紹與範例 |
| **AN5711** - Getting Started with STM32H5 Hardware | [PDF](https://www.st.com/resource/en/application_note/an5711-getting-started-with-stm32h5-mcu-hardware-development-stmicroelectronics.pdf) | 硬體開發指南 |
| STM32H563ZI 官方產品頁 | [st.com](https://www.st.com/en/evaluation-tools/nucleo-h563zi.html) | 開發板資訊 |
| STM32H563 Reference Manual | TBD | 晶片詳細技術文件 |

---

## 💻 編譯與燒錄工具

### 官方工具

| 工具 | 下載連結 | 說明 |
|------|----------|------|
| **STM32CubeIDE** | st.com | 官方 IDE，支援編譯 + 燒錄 + Debug |
| **STM32CubeProgrammer** | st.com | 獨立燒錄工具 |
| **STM32CubeH5** (FW Package) | GitHub | 韌體套件，含 HAL/LL |
| **ST-Link Driver** | st.com | NUCLEO 內建 Debugger 驅動 |

### NUCLEO-H563ZI 特性
- 內建 STLINK-V3EC debugger/programmer
- USB Device/Host 全速
- I3C 介面
- 250 MHz Cortex-M33

---

## 🧪 I3C 相關範例

### 官方範例 (STM32CubeH5)

**GitHub:** https://github.com/STMicroelectronics/STM32CubeH5

```
Projects/NUCLEO-H563ZI/Examples/I3C/
├── I3C_Controller_Switch_To_Target/  # 主要範例
├── I3C_Target_BCR_DCR_Read/          # 讀取裝置資訊
└── I3C_HotJoin/                      # Hot Join 程序
```

### 社群參考

| 討論主題 | 連結 |
|----------|------|
| H563ZI I3C GETACCCR 問題與解答 | [ST Community](https://community.st.com/t5/stm32-mcus-products/h563zi-i3c-getacccr/td-p/663066) |
| AN5879 + NUCLEO-H563ZI + XNUCLEO-IKS4A1 | [ST Community](https://community.st.com/t5/stm32-mcus-boards-and-hardware/an5879-i3c-for-nucleo-h563zi-and-xnucleo-iks4a1/td-p/778889) |
| STM32H5 I3C HAL Library 使用方式 | [ST Community](https://community.st.com/t5/stm32-mcus-products/how-to-use-the-hal-i3c-library/td-p/569606) |

---

## 🔌 USB 開發資源

### USB HID

| 資源 | 連結 | 說明 |
|------|------|------|
| **STSW-STM32084** - USB HID Demonstrator | [st.com](https://www.st.com/en/development-tools/stsw-stm32084.html) | 官方 USB HID 範例 |
| Custom HID Class 實作 (Part 1) | [ST Community](https://community.st.com/t5/stm32-mcus/how-to-implement-a-usb-device-custom-hid-class-on-stm32-part-1/ta-p/49423) | 逐步教學 |
| Custom HID 逐步教學 | [iopush.net](https://notes.iopush.net/blog/2016/stm32-custom-usb-hid-step-by-step-2/) | 詳細範例 |
| USB CDC + HID Composite | [ST Community](https://community.st.com/t5/stm32-mcus/how-to-implement-the-usb-device-composite-class-usb-dfu-hid/ta-p/759762) | 複合裝置 |

### USB 相關問題

| 問題 | 連結 |
|------|------|
| STM32H563 USB HID Device 枚举失敗 | [ST Community](https://community.st.com/t5/stm32-mcus-embedded-software/stm32h563-usb-hid-device-behind-hub-not-enumerated/td-p/620228) |

---

## 🛠️ 第三方範例與工具

### 範例專案

| 專案 | 連結 | 說明 |
|------|------|------|
| binod1780/Nucleo-H563ZI | [GitHub](https://github.com/binod1780/Nucleo-H563ZI) | 範例專案集合 |
| Serasidis/STM32_HID_Bootloader | [GitHub](https://github.com/Serasidis/STM32_HID_Bootloader) | HID bootloader |

### RTOS 支援

| 平台 | 連結 |
|------|------|
| **Zephyr RTOS** - NUCLEO-H563ZI | [Docs](https://docs.zephyrproject.org/latest/boards/st/nucleo_h563zi/doc/index.html) |

---

## 📋 開發流程建議

### 1. 環境建置
1. 下載安裝 STM32CubeIDE
2. 下載 STM32CubeH5 FW Package
3. 安裝 ST-Link 驅動

### 2. 驗證範例
1. 打開 `I3C_Controller_Switch_To_Target` 範例
2. 編譯並燒錄到 NUCLEO-H563ZI
3. 確認 I3C 通訊正常

### 3. USB HID 實作
1. 参考 STSW-STM32084 USB HID Demonstrator
2. 修改 Report Descriptor
3. 實現資料傳輸

### 4. 整合 I3C + USB
1. I3C 作為前端 (連接感測器)
2. USB HID 作為後端 (連接 PC)
3. 實作資料轉發

---

## 📌 待購買硬體

- [ ] NUCLEO-H563ZI 開發板
- [ ] X-NUCLEO-IKS4A1 (I3C 感測器擴充板)
- [ ] USB Type-C 線材

---

## 🔗 快速連結

- ST I3C 官方教學: https://www.st.com/resource/en/application_note/an5879
- STM32H5 官方網站: https://www.st.com/en/microcontrollers-mcus/stm32h5-series.html
- STM32CubeH5 GitHub: https://github.com/STMicroelectronics/STM32CubeH5
