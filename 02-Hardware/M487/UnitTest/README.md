# M487_ScsiTool 單元測試知識庫 (Unit Test Knowledge Base)

> **專案**: M487_ScsiTool - Nuvoton M487 USB 儲存裝置控制韌體  
> **目標**: 建立完整的嵌入式 C 單元測試知識庫  
> **维护**: Tamama (知識庫與研究)  
> **更新日期**: 2026-04-07

---

## 📁 知識庫結構

```
M487/UnitTest/
├── Unity/                    # Unity 框架（嵌入式 C 單元測試）
│   ├── README.md             # Unity 概述與快速入門
│   ├── M487_Examples/        # M487 專用範例
│   │   ├── test_i2c_driver.c # I2C 驅動程式測試
│   │   ├── test_hid_parser.c # HID 解析器測試
│   │   └── test_uart_debug.c # UART 偵錯日誌測試
│   └── Best_Practices.md     # 最佳實踐指南
├── Ceedling/                 # Ceedling 框架（Unity + CMock）
│   ├── README.md             # Ceedling 概述與設定
│   ├── M487_Examples/        # Ceedling M487 專案
│   └── Integration_Guide.md   # 整合指南
├── GoogleTest/               # GoogleTest（主機端工具測試）
│   ├── README.md             # GoogleTest 概述
│   ├── M487_HostTools/       # 主機工具測試（hidtool.py, msc_debug.exe）
│   └── Mock_USB_HID.md       # USB HID Mock 技術
├── Strategy/                 # 測試策略
│   ├── M487_Testing_Strategy.md  # M487 測試策略
│   ├── Test_Coverage_Matrix.md    # 測試覆蓋率矩陣
│   └── CI_Integration.md          # CI 整合
└── README.md                 # 本檔案
```

---

## 🎯 框架選擇決策指南

| 場景 | 推薦框架 | 原因 |
|------|----------|------|
| **韌體模組**（無作業系統） | Unity | 輕量、純 C、高可移植性 |
| **需要 Mock 複雜依賴** | Ceedling (Unity + CMock) | 自動生成 Mock、隔離測試 |
| **主機端工具**（Windows） | GoogleTest | C++、豐富功能、Windows 整合佳 |
| **跨平台主機工具** | GoogleTest | 跨平台、社群支援完善 |

### 詳細決策樹

```
你的測試目標是什麼？
├── 嵌入式韌體（無作業系統）
│   ├── 需要 Mock 硬體依賴 → Ceedling
│   └── 簡單邏輯測試 → Unity
└── 主機端工具/桌面應用
    └── GoogleTest
```

---

## 📚 各框架簡介

### Unity
- **用途**: 嵌入式 C 程式碼單元測試
- **特點**: 純 C、輕量（約 3 個檔案）、高可移植性
- **官方網站**: https://www.throwtheswitch.org/unity
- **適用場景**: M487 I2C/SPI/GPIO 驅動程式測試

### Ceedling
- **用途**: TDD（測試驅動開發）自動化框架
- **特點**: Unity + CMock + CException 整合
- **官方網站**: https://www.throwtheswitch.org/ceedling
- **適用場景**: 需要 Mock 複雜硬體抽象層的專案

### GoogleTest
- **用途**: C++ 單元測試框架
- **特點**: 豐富的斷言、Mock 支援、測試過濾
- **官方網站**: https://google.github.io/googletest/
- **適用場景**: hidtool.py、msc_debug.exe 等主機工具測試

---

## 🔧 M487 專用測試範例

### 1. I2C 驅動程式測試
測試 M487 I2C 暫存器讀寫與 NACK 重試邏輯。

### 2. HID 解析器測試
測試 USB HID 描述符解析與報告 ID 匹配。

### 3. UART 偵錯日誌測試
測試格式化輸出與日誌層級處理。

### 4. USB MSC 協議測試
測試 SCSI 命令解析與 USB 傳輸狀態機。

---

## 📖 文件索引

| 文件 | 說明 |
|------|------|
| [Unity/README.md](./Unity/README.md) | Unity 框架完整指南 |
| [Ceedling/README.md](./Ceedling/README.md) | Ceedling 框架完整指南 |
| [GoogleTest/README.md](./GoogleTest/README.md) | GoogleTest 完整指南 |
| [Strategy/M487_Testing_Strategy.md](./Strategy/M487_Testing_Strategy.md) | M487 測試策略 |
| [Strategy/Test_Coverage_Matrix.md](./Strategy/Test_Coverage_Matrix.md) | 測試覆蓋率目標 |
| [Strategy/CI_Integration.md](./Strategy/CI_Integration.md) | CI/CD 整合 |

---

## 🚀 快速開始

### 1. 選擇適合的框架
根據上方的決策指南選擇框架。

### 2. 設定測試環境
```bash
# Unity - 直接下載或使用子模組
git submodule add https://github.com/ThrowTheSwitch/Unity.git Unity

# Ceedling - 需要 Ruby 環境
gem install ceedling

# GoogleTest - 使用 vcpkg 或 CMake FetchContent
```

### 3. 執行測試
```bash
# Unity - 編譯並執行測試_runner
gcc -IUnity -c unity.c test_*.c src/*.c -o test_runner && ./test_runner

# Ceedling
ceedling test:all

# GoogleTest
cmake --build build && ctest
```

---

## 📝 貢獻指南

1. 所有文件使用**繁體中文**
2. 程式碼註解使用**英文**
3. 提交前確認所有範例可編譯執行
4. 使用 `git commit` 並描述變更內容

---

## 🔗 外部資源

- [Unity 官方文檔](https://github.com/ThrowTheSwitch/Unity/blob/master/docs/UnityGettingStartedGuide.md)
- [Ceedling 官方文檔](https://github.com/ThrowTheSwitch/ceedling)
- [GoogleTest 官方文檔](https://google.github.io/googletest/)
- [ThrowTheSwitch 部落格](https://www.throwtheswitch.org/)

---

**下次更新計畫**:
- 新增 M487 USB MSC 協議測試範例
- 新增 Mock GPIO 中斷處理範例
- 新增 CI/CD 完整配置範例

---
