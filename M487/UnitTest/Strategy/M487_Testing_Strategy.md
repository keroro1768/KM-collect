# M487 測試策略 (M487 Testing Strategy)

> 本文件定義 M487_ScsiTool 韌體專案的測試策略，包括測試目標、分層架構、和執行方法。

---

## 📑 目錄

1. [測試目標](#測試目標)
2. [測試金字塔](#測試金字塔)
3. [測試分層架構](#測試分層架構)
4. [測試方法](#測試方法)
5. [何時使用何種測試](#何時使用何種測試)
6. [測試環境](#測試環境)
7. [風險管理](#風險管理)

---

## 測試目標

### 主要目標

| 目標 | 說明 | 量化指標 |
|------|------|----------|
| **功能正確性** | 韌體功能符合規格 | 100% 核心功能測試覆蓋 |
| **穩定性** | 長時間運行無崩潰 | 100 小時壓力測試 |
| **效能** | 符合效能要求 | USB 傳輸 ≥ 10 MB/s |
| **安全性** | 無記憶體洩漏/溢出 | 0 個靜態分析警告 |

### 次要目標

- 快速反饋：本地測試 < 5 秒
- CI 整合：每次 commit 自動測試
- 可重現性：相同程式碼，相同結果

---

## 測試金字塔

```
                    ┌──────────────────────┐
                    │   System Tests       │  ← 5%
                    │   (Integration)      │
                    ├──────────────────────┤
                    │   Integration Tests  │  ← 15%
                    │   (Module)          │
                    ├──────────────────────┤
                    │   Unit Tests        │  ← 80%
                    │   (Function)        │
                    └──────────────────────┘
```

### 各層測試比例

| 層級 | 比例 | 工具 | 執行頻率 |
|------|------|------|----------|
| **單元測試** | 80% | Unity / Ceedling | 每次 commit |
| **整合測試** | 15% | Ceedling | 每次 PR |
| **系統測試** | 5% | 手動 / 自動化腳本 | 每次 Release |

---

## 測試分層架構

### M487 韌體分層

```
┌─────────────────────────────────────────────────┐
│           Application Layer                       │
│   (SCSI Command Handler, File System)            │
├─────────────────────────────────────────────────┤
│           Protocol Layer                         │
│   (USB MSC, SCSI, FAT)                          │
├─────────────────────────────────────────────────┤
│           Driver Layer                           │
│   (USB Device, Storage, SPI Flash)              │
├─────────────────────────────────────────────────┤
│           HAL Layer                             │
│   (GPIO, I2C, UART, Timer)                      │
├─────────────────────────────────────────────────┤
│           Hardware (M487 Cortex-M4)              │
└─────────────────────────────────────────────────┘
```

### 各層測試策略

#### 1. HAL 層測試

| 測試對象 | 測試內容 | 工具 |
|---------|---------|------|
| GPIO | 讀取/寫入/中斷 | Unity + Mock |
| I2C | 傳輸/暫存器讀寫 | Unity + Mock |
| UART | 發送/接收/緩衝區 | Unity |
| Timer | 計時/中斷觸發 | Unity + Mock |

#### 2. Driver 層測試

| 測試對象 | 測試內容 | 工具 |
|---------|---------|------|
| USB Device | 列舉/傳輸/端點 | Ceedling Mock |
| SPI Flash | 讀/寫/抹除 | Unity |
| NAND Flash | 壞塊管理 | Unity |

#### 3. Protocol 層測試

| 測試對象 | 測試內容 | 工具 |
|---------|---------|------|
| USB MSC | 命令解析/狀態機 | Ceedling Mock |
| SCSI | 命令驗證/回應 | Ceedling |
| FAT | 目錄/檔案操作 | Ceedling |

#### 4. Application 層測試

| 測試對象 | 測試內容 | 工具 |
|---------|---------|------|
| Command Parser | 參數解析/錯誤處理 | Ceedling |
| State Machine | 狀態轉換 | Ceedling |
| Buffer Manager | 記憶體分配 | Unity |

---

## 測試方法

### 離線測試（Off-Target）

韌體程式碼在主機（Windows/Linux）上編譯和測試，不需要實際硬體。

**優點**：
- 執行速度快
- 不需要硬體
- 調試方便
- CI 整合簡單

**缺點**：
- 無法測試實際硬體
- 時序行為可能不同

**適用場景**：
- 80% 的單元測試
- 協定邏輯測試
- 演算法測試

### 在線測試（On-Target）

程式碼在實際 M487 硬體上執行測試。

**優點**：
- 測試真實硬體行為
- 發現硬體相關問題
- 時序精確

**缺點**：
- 需要硬體設備
- 執行速度慢
- 調試困難

**適用場景**：
- HAL 驅動程式驗證
- 中斷處理測試
- USB 協定測試

### 混合測試

部分模組在主機測試，部分在目標測試。

```
┌─────────────────┐     ┌─────────────────┐
│   Host Test     │     │   Target Test    │
│                 │     │                 │
│  Protocol       │     │  USB Device     │
│  SCSI           │     │  I2C            │
│  FAT            │     │  SPI Flash      │
│  Buffer Mgr     │     │  Interrupts     │
└─────────────────┘     └─────────────────┘
```

---

## 何時使用何種測試

### 決策樹

```
開始
  │
  ├─ 是否需要硬體？
  │     │
  │     ├─ 是 ──→ 在線測試 (On-Target)
  │     │         └─ HAL, 中斷, USB
  │     │
  │     └─ 否 ──→ 需要 Mock 複雜依賴？
  │               │
  │               ├─ 是 ──→ Ceedling + CMock
  │               │         └─ 協定、狀態機
  │               │
  │               └─ 否 ──→ Unity 純 Mock
  │                         └─ 簡單邏輯、工具函式
```

### 框架選擇矩陣

| 模組 | 測試類型 | 框架 | Mock 層級 |
|------|----------|------|-----------|
| I2C 驅動 | 離線 | Unity | Mock HAL |
| USB MSC | 離線 | Ceedling | Mock USB Device |
| SCSI 解析 | 離線 | Ceedling | Mock Storage |
| HID Parser | 離線 | Unity | Mock USB EP |
| UART Debug | 離線 | Unity | Mock UART |
| FAT FS | 離線 | Ceedling | Mock Block Device |
| Buffer Mgr | 離線 | Unity | 無 |
| 整合測試 | 離線 | Ceedling | Mock All |
| HAL Init | 在線 | 手動 | 無 |

---

## 測試環境

### 本地開發環境

```
┌────────────────────────────────────────────┐
│            Windows 開發環境                 │
├────────────────────────────────────────────┤
│                                            │
│  ┌──────────┐  ┌──────────┐  ┌─────────┐ │
│  │ Unity    │  │ Ceedling  │  │ GTest   │ │
│  │ 離線測試  │  │ 離線測試  │  │ 主機工具 │ │
│  └────┬─────┘  └────┬─────┘  └────┬────┘ │
│       │             │             │       │
│       └──────┬──────┴──────┬──────┘       │
│              │             │              │
│       ┌──────┴─────────────┴──────┐      │
│       │      Mock USB HID          │      │
│       │      Mock I2C              │      │
│       │      Mock GPIO             │      │
│       └────────────────────────────┘      │
│                                            │
└────────────────────────────────────────────┘
```

### CI 環境

```yaml
# GitHub Actions
jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Install Ruby
        uses: ruby/setup-ruby@v1
        with:
          ruby-version: 3.2
      - name: Install Ceedling
        run: gem install ceedling
      - name: Run unit tests
        run: ceedling test:all
      - name: Run coverage
        run: ceedling coverage
```

---

## 風險管理

### 測試覆蓋率缺口

| 風險 | 影響 | 緩解措施 |
|------|------|----------|
| USB 時序問題 | 高 | 在線測試 |
| 中斷競爭條件 | 高 | 程式碼審查 |
| Flash 壞塊 | 中 | 模擬測試 |
| 記憶體碎片 | 中 | 壓力測試 |
| 邊界條件 | 低 | 單元測試覆蓋 |

### 測試通過標準

| 指標 | 目標 |
|------|------|
| 單元測試覆蓋率 | ≥ 80% |
| 分支覆蓋率 | ≥ 70% |
| 測試通過率 | 100% |
| CI 成功率 | ≥ 95% |

---

## 延伸閱讀

- [測試覆蓋率矩陣](./Test_Coverage_Matrix.md)
- [CI 整合](./CI_Integration.md)
- [Unity 最佳實踐](../Unity/Best_Practices.md)

---
