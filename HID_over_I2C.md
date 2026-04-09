# HID over I2C 知識筆記

## 1. 概述 (Overview)
**HID over I2C** 是一種將人類界面設備 (Human Interface Device, HID) 協議運行在 I2C 總線上的實現方案。它允許如觸控板 (Touchpad)、觸控螢幕 (Touchscreen)、感測器 (Sensors) 及鍵盤等設備透過低功耗的 I2C 接口與主機通訊。

- **核心目的**：將原本為 USB/藍牙設計的 HID 協議移植到 I2C 總線，降低功耗並減少接線。
- **Windows 驅動**：主要由內建的 `HIDI2C.sys` (KMDF 驅動) 支援。
- **通用性**：該協議本身是總線不可知 (Bus-agnostic) 的，因此可以移植到多種傳輸層。

---

## 2. Windows 系統架構 (Windows Architecture)

### 2.1 驅動堆疊 (Driver Stack)
Windows 使用 **SPB (Simple Peripheral Bus)** 框架來管理低功耗總線（如 I2C 和 SPI）。

- **HIDI2C.sys (HID Miniport Driver)**：實現 HID over I2C 協議 v1.0，處理 HID IOCTLs 並提供應用層兼容性。
- **SPB Class Extension**：處理與資源中心 (Resource Hub) 的交互，管理同步目標的隊列。
- **I2C Controller Driver**：提供實際的控制器底層實現，暴露 SPB IOCTL 接口進行讀寫操作。
- **GPIO Controller Driver**：負責將設備的中斷信號 (Interrupts) 傳遞給 Windows。

### 2.2 關鍵組件
- **Resource Hub (資源中心)**：由於 SoC 平台上的設備通常不可發現 (Non-discoverable)，設備必須在 **ACPI** 中靜態定義。資源中心作為代理，將 `HIDI2C` 驅動的請求重新路由到正確的控制器驅動。
- **ACPI 資源要求**：
  - 必須定義 `HID I2C connection`。
  - 必須定義 `Device interrupt`。

---

## 3. Linux 實現 (Linux Implementation)

Linux 透過 `i2c-hid` 內核模組提供支援。

### 3.1 Device-Tree 配置
在 Linux 設備樹中，定義 HID over I2C 設備需要以下屬性：
- **compatible**: 必須包含 `"hid-over-i2c"`。
- **reg**: I2C 從機地址 (Slave Address)。
- **hid-descr-addr**: HID 描述符地址。
- **interrupts**: 中斷線路。

**範例：**
```dts
i2c-hid-dev@2c {
    compatible = "hid-over-i2c";
    reg = <0x2c>;
    hid-descr-addr = <0x0020>;
    interrupt-parent = <&gpx3>;
    interrupts = <3 2>;
};
```

---

## 4. 協議特點 (Protocol Features)
- **描述符與報告 (Descriptors & Reports)**：遵循標準 HID 規範，使用報告描述符來定義設備能力。
- **電源管理 (Power Management)**：支援低功耗模式與喚醒機制。
- **適用場景**：最常用於筆記型電腦的 I2C 觸控板與環境感測器。

---

## 5. 參考資源
- [Microsoft Learn: Introduction to HID Over I2C](https://learn.microsoft.com/en-us/windows-hardware/drivers/hid/hid-over-i2c-guide)
- [Microsoft Learn: Architecture and Overview](https://learn.microsoft.com/en-us/windows-hardware/drivers/hid/architecture-and-overview)
- [Kernel.org: HID over I2C Device-Tree bindings](https://www.kernel.org/doc/Documentation/devicetree/bindings/input/hid-over-i2c.txt)
