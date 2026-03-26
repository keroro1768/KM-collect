# USB-I3C Bridge 專案工作日誌

> 建立日期: 2026-03-26
> 最後更新: 2026-03-26 22:53
> 檢查頻率: 每 30 分鐘

---

## 任務狀態總覽

| # | 任務 | 狀態 | 優先級 |
|---|------|------|--------|
| T1 | 硬體調研 (STM32H563ZI) | ✅ 完成 | P0 |
| T2 | 開發工具鏈調研 | ✅ 完成 | P0 |
| T3 | USB HID 範例程式碼整理 | ✅ 完成 | P1 |
| T4 | I2C 範例程式碼整理 | ✅ 完成 | P1 |
| T5 | I3C 範例程式碼整理 | ✅ 完成 | P1 |
| T6 | USB-I3C Bridge 韌體架構設計 | ✅ 完成 | P1 |
| T7 | 建立 KM 文件 | ✅ 完成 | P0 |
| **T8** | **系統架構設計 (MSC/CDC/HID)** | ✅ 完成 | P0 |
| **T8b** | **現有開源專案調研** | ✅ 完成 | P0 |
| T9 | 硬體採購 (NUCLEO-H563ZI) | ⏳ 待處理 | P0 |
| T10 | 安裝開發環境 (PlatformIO) | ⏳ 待處理 | P0 |
| T11 | USB MSC 驅動程式開發 | ⏳ 待處理 | P1 |
| T12 | USB CDC ACM 驅動程式開發 | ⏳ 待處理 | P1 |
| T13 | Vendor HID 驅動程式開發 | ⏳ 待處理 | P1 |
| T14 | I3C 通訊框架實作 | ⏳ 待處理 | P1 |
| T15 | I2C 通訊框架實作 | ⏳ 待處理 | P1 |
| T16 | BridgeCLI 測試工具開發 | ⏳ 待處理 | P2 |
| T17 | BridgeTool GUI 測試工具開發 | ⏳ 待處理 | P2 |
| T18 | I3C Client/Targer 端程式碼 | ⏳ 待處理 | P2 |
| T19 | 功能測試與整合 | ⏳ 待處理 | P2 |

---

## Phase 規劃

### Phase 1: 基本通訊 (MVP)
- T10, T12, T14, T15
- USB CDC ACM + I3C 讀寫
- 文字命令列介面

### Phase 2: 強化功能
- T11, T13
- USB MSC 介面
- Vendor HID

### Phase 3: 測試工具
- T16, T17
- BridgeCLI / BridgeTool

### Phase 4: Client 端
- T18
- I3C Target 端程式碼

---

## 每日進度記錄

### 2026-03-26 (Day 1)

**22:00 - 22:30**
- [x] 在 KM 中搜尋 USB to I3C Bridge 相關資料
- [x] 找到 ST STM32H563ZI 開發資源
- [x] 確認 NUCLEO-H563ZI 開發板資訊

**22:30 - 22:45**
- [x] 網路搜尋 I3C + USB 開發相關資源
- [x] 找到 AN5879, AN5711 文件
- [x] 找到 STM32CubeH5 GitHub 範例
- [x] 找到 USB HID + CDC Composite 範例

**22:45 - 22:48**
- [x] 建立 KM 文件
  - `04-STM32H563ZI-Development.md`
  - `05-AN5879-I3C-STM32H5-Tutorial.md`
  - `06-USB-I3C-Bridge-Firmware.md`
- [x] 完成 CLI 工具鏈調研 (推薦 PlatformIO)
- [x] 完成 USB/I2C/I3C 範例程式碼整理
- [x] 完成 USB-I3C Bridge 韌體架構設計

**22:48 - 22:53**
- [x] 建立工作日誌追蹤系統 (`00-Project-Tasks.md`)
- [x] 新增系統架構設計 (`07-System-Architecture.md`)
- [x] 完成三種 USB 介面設計 (MSC/CDC/HID)
- [x] 完成 I3C Client 端設計說明
- [x] 完成測試工具規劃 (BridgeCLI/BridgeTool/BridgeStorage)

**22:53 - 22:58**
- [x] 搜尋現有 USB→I3C 開源專案
- [x] 找到 **I2C-Tiny-USB** (最相關!)
  - 基於 ATtiny45，Linux kernel driver
  - 可被 lm_sensors 使用
- [x] 找到 **i2c-star** (STM32F103 版本)
- [x] 找到 Linux Kernel I3C Subsystem 文件
- [x] 找到 CH341 USB to I2C driver
- [x] 找到商業產品 (Binho, MCCI, PGY)

---

## 下一個工作時段規劃

### 選項 A: 若有硬體 (NUCLEO-H563ZI)
1. 安裝 PlatformIO
2. 下載 STM32CubeH5 FW Package
3. 嘗試編譯 USB CDC + I3C 範例
4. 實現基礎通訊

### 選項 B: 若無硬體 (軟體調研持續進行)
1. 研究 USB MSC Class 程式碼
2. 研究 USB CDC ACM Class 程式碼
3. 設計 BridgeCLI 命令格式
4. 撰寫 I3C Target 端範例程式碼
5. 優化系統架構文件

---

## 備註

- **檢查頻率**: 每 30 分鐘 (整點+30分)
- **下次檢查時間**: 23:00 → 23:30 → 00:00...
- 文件位置: `D:\Project\KM-collect\Hardware\I3C-USB-Bridge\`
- 目前已完成 Phase 1 前期調研，架構設計完成
- 等待硬體購置後可進行實際開發
