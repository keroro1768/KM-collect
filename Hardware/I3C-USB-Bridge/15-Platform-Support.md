# USB-I3C Bridge 平台支援調查

> 建立日期: 2026-03-27
> 任務: T23

---

## 📊 平台支援總覽

| 平台 | 支援狀態 | 驅動程式 | 備註 |
|------|----------|----------|------|
| **Linux** | ✅ 完整支援 | 內建 (mipi-i3c-hci) | Kernel 5.x+ |
| **Windows** | 🔄 開發中 | USB-IF 推動中 | 預計未來支援 |
| **macOS** | 🔄 開發中 | USB-IF 推動中 | 預計未來支援 |
| **Raspberry Pi** | ✅ 支援 | 依賴 Linux kernel | 需要啟用 I3C |

---

## 🔬 詳細分析

### 1. Linux 支援

**狀態:** ✅ 完整支援

**內建驅動:**
- `mipi-i3c-hci` - MIPI I3C Host Controller Interface
- `i3c-master` - I3C 主控驅動
- `/dev/i3c-*` - 裝置節點

**Kernel 配置:**
```
CONFIG_I3C=y           # I3C support
CONFIG_MIPI_I3C_HCI=y  # MIPI I3C HCI driver
```

**使用方式:**
```bash
# 列出 I3C 裝置
ls -la /dev/i3c-*

# 使用 i2c-tools 存取
i2cdetect -l  # 會顯示 I3C 匯流排
```

**相關程式碼:**
- `drivers/i3c/master.c` (Linux kernel)
- `drivers/i3c/device.c`

**支援的硬體:**
- Synopsys DesignWare I3C IP
- AMD I3C IP
- Various SoC 內建 I3C

---

### 2. Windows 支援

**狀態:** 🔄 開發中

**最新進展 (2025年2月):**
- USB-IF 發布了 I3C Device Class Specification
- Binho LLC 參與制定標準
- Windows 驅動程式正在開發中

**目前可用方案:**
| 方案 | 說明 |
|------|------|
| **libusbK** | 通用 USB 驅動庫，可存取自訂 USB 裝置 |
| **WinUSB** | Microsoft 提供的標準 USB 驅動 |
| **自建驅動** | 需要開發 KMDF 驅動程式 |

**開發選項:**
```
1. 使用 WinUSB + libusbdotnet 存取
2. 使用 HID class (如果支援)
3. 自建 KMDF 驅動程式
```

**相關資源:**
- libusbK: https://sourceforge.net/projects/libusbK/
- libwdi: https://github.com/pbatard/libwdi

---

### 3. macOS 支援

**狀態:** 🔄 開發中

**USB-IF 規劃:**
- 與 Windows 驅動同步開發
- 目標是納入 OS 官方驅動

**目前可用方案:**
- **libusb** (Homebrew): `brew install libusb`
- **IOKit** 直接存取

**Swift 範例:**
```swift
import IOKit
import IOKit.usb
// 直接存取 USB 裝置
```

---

### 4. Raspberry Pi 支援

**狀態:** ✅ 支援 (依賴 Linux)

**硬體連接:**
- Raspberry Pi 5 支援 I3C
- 需要啟用 DT overlay

**啟用 I3C:**
```bash
# /boot/firmware/config.txt
dtparam=i2c_vc=on
dtparam=i3c=on
```

**可用腳位:**
| 功能 | Pin | 備註 |
|------|-----|------|
| I3C SCL | Pin 3 | 可選 |
| I3C SDA | Pin 5 | 可選 |

**相關討論:**
- Raspberry Pi 官方論壇有 I3C 啟用討論
- 需要編譯自訂 kernel 或使用最新 OS 版本

---

## 📡 USB-I3C 轉換器市場

### 現有產品

| 產品 | 平台支援 | 備註 |
|------|----------|------|
| **Binho I3C USB Host Adapter** | 開源驅動 | Windows/macOS/Linux |
| **I²CDriver** | 跨平台 (Python) | 開源 GUI |
| **Tag-Connect TC2030** | 需自建 | 硬體方案 |

### Binho 產品特色
- USB to I3C 轉換器
- 支援 Windows/macOS/Linux
- 提供開源 Python 庫
- 符合 USB-IF I3C Device Class 標準

---

## 🏗️ 我們的專案策略

### 短期方案 (立即可做)

**USB CDC ACM 驅動:**
- 所有 OS 都支援 (内建)
- 無需額外驅動程式
- 傳輸速率較慢

**USB HID 驅動:**
- Windows/macOS/Linux 内建
- 適合中低速傳輸

### 中期方案 (等待 USB-IF)

**USB-IF I3C Device Class:**
- 標準化驅動程式
- 預計支援 Windows/macOS/Linux
- 可獲得正式驅動支援

### 長期方案 (自建)

**libusbK 自訂:**
- 可跨平台使用
- 需要較多開發工作
- 完全控制

---

## 📋 待確認事項

- [ ] 等待 USB-IF I3C Device Class 正式發布
- [ ] 評估是否採用 Binho 或類似轉換器
- [ ] 開發自訂 libusb 客戶端程式

---

## 📎 參考資源

- [Linux I3C Subsystem](https://docs.kernel.org/driver-api/i3c/index.html)
- [MIPI I3C FAQ](https://www.mipi.org/resources/i3c-frequently-asked-questions)
- [USB-IF I3C Device Class (Binho)](https://binho.io/blogs/i3c-articles/usb-if-releases-i3c-device-class-specification)
- [libusbK](https://sourceforge.net/projects/libusbK/)
- [Linux I3C master.c](https://github.com/torvalds/linux/blob/master/drivers/i3c/master.c)
