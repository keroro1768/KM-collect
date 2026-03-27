# USB-I3C Bridge 開發板選用建議

> 提案日期: 2026-03-27
> 提案人: Caro

---

## 📌 結論

**建議採用: NUCLEO-H563ZI (STM32H563ZIT6)**

| 項目 | 說明 |
|------|------|
| **單價** | ~NT$1,200 (含 Debugger) |
| **供應商** | STMicroelectronics |
| **交期** | 現貨供應 |

---

## 🎯 專案需求 vs 開發板能力

### 需求對照

| 需求 | NUCLEO-H563ZI 支援 | 備註 |
|------|-------------------|------|
| **I3C 通訊** | ✅ 原生 I3C 控制器 | 內建硬體 I3C，符合 MIPI I3C 標準 |
| **USB Device** | ✅ USB Full-Speed | 支援 HID, CDC, Composite |
| **Debug/燒錄** | ✅ 內建 STLINK-V3EC | 無需額外工具 |
| **Cortex-M33 效能** | ✅ 250 MHz | 足夠處理資料轉發 |
| **低功耗** | ✅ ST 節能技術 | 支援 Sleep/Stop 模式 |
| **開源生態** | ✅ STM32CubeHAL | 官方完整支援 |

---

## 🔬 技術亮點

### 1. 原生 I3C 支援 (AN5879)
- ST 官方提供完整 I3C 教學文件 (AN5879)
- 已有現成範例可直接修改:
  - `I3C_Controller_Switch_To_Target`
  - `I3C_HotJoin` (熱插入支援)
- GitHub: `STMicroelectronics/STM32CubeH5`

### 2. USB HID 免驅動
- Windows/macOS/Linux 內建 HID 驅動
- 無需安裝額外驅動程式
- 符合 HID-over-I3C 發展趨勢

### 3. 完整開發生態
| 工具 | 用途 |
|------|------|
| STM32CubeIDE | 編譯 + Debug |
| STM32CubeProgrammer | 獨立燒錄 |
| STM32CubeH5 | HAL/LL 驅動程式 |

---

## 💰 成本分析

### 硬體成本
| 項目 | 數量 | 單價 (NT$) | 小計 |
|------|------|-----------|------|
| NUCLEO-H563ZI | 1 | 1,200 | 1,200 |
| USB Type-C 線 | 1 | 200 | 200 |
| **總計** | | | **~1,400** |

### 相較其他方案
| 方案 | 單價 (NT$) | 備註 |
|------|-----------|------|
| **NUCLEO-H563ZI** | ~1,200 | 原生 I3C + 內建 Debugger |
| Raspberry Pi Pico | ~150 | 需外接 I3C 電路 |
| 自行設計 PCB | ~500+ | 需Debug工具 + 加工時間 |

---

## 📦 採購清單

| 項目 | 型號 | 數量 |
|------|------|------|
| 開發板 | NUCLEO-H563ZI | 1 |
| USB Type-C 線 | - | 1 |

---

## ✅ 預期成果

1. **短期 (1-2週)**: 完成 USB-I3C Bridge 韌體開發
2. **中期 (1個月)**: 整合 USB HID 通訊協定
3. **長期**: 形成可量產之 USB-I3C 轉換器模組

---

## 📎 參考文件

- AN5879 - I3C Introduction for STM32H5
- NUCLEO-H563ZI 官方網頁
- STM32CubeH5 GitHub Repository
