# HID Over I2C 技術筆記

## 概述

HID over I2C（HID-I2C）是一種讓人機介面裝置（如鍵盤、滑鼠、觸控板）透過 I2C 匯流排與作業系統通訊的協定。

![HID Over I2C 架構圖](./images/hadimg_hid_i2c_1.jpg)
> 圖說：FocalTech FT6336G 觸控螢幕控制器晶片，廣泛應用於筆記型電腦的觸控板

## 主要特點

- **低功耗**：相比 USB 更適合行動裝置
- **標準化**：Microsoft 主導的開放標準
- **广泛支援**：Linux、Windows、macOS 都有原生支援

## 運作原理

1. 裝置透過 I2C 匯流排與主機通訊
2. HID 報告描述了裝置的功能
3. 作業系統載入對應的驅動程式

## 應用場景

- 筆電觸控板
- 觸控螢幕
- 感測器集線器

## 參考資源

- [Microsoft 官方文檔](https://learn.microsoft.com/en-us/windows-hardware/drivers/hid/hid-over-i2c-guide)
- [Linux Kernel I2C-HID](https://github.com/torvalds/linux/blob/master/drivers/hid/i2c-hid/)