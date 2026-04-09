# Ceedling Framework - 嵌入式 C 自動化測試框架

> **框架**: ThrowTheSwitch/Ceedling  
> **用途**: 整合 Unity + CMock + CException 的自動化 TDD 框架  
> **官方網站**: https://www.throwtheswitch.org/ceedling  
> **GitHub**: https://github.com/ThrowTheSwitch/ceedling

---

## 📋 目錄

1. [概述](#概述)
2. [特色與優勢](#特色與優勢)
3. [安裝與設定](#安裝與設定)
4. [專案結構](#專案結構)
5. [設定檔案](#設定檔案)
6. [Mock 與 Stub](#mock-與-stub)
7. [M487 整合](#m487-整合)
8. [常見命令](#常見命令)
9. [最佳實踐](#最佳實踐)

---

## 概述

Ceedling 是一個專為 C 語言專案設計的自動化測試框架，它整合了三個核心元件：

| 元件 | 用途 |
|------|------|
| **Unity** | 單元測試框架 |
| **CMock** | 自動 Mock 生成框架 |
| **CException** | 異常處理框架 |

Ceedling 通過 Ruby 自動化構建流程，讓 TDD（測試驅動開發）變得簡單高效。

---

## 特色與優勢

### 為什麼選擇 Ceedling？

| 特性 | 說明 |
|------|------|
| **自動 Mock 生成** | 根據 Header 自動生成 Mock 函式 |
| **零配置** | `ceedling new` 即可建立完整專案 |
| **依賴管理** | 自動處理測試依賴關係 |
| **多目標測試** | 支援主機測試與目標測試 |
| **代碼覆蓋率** | 內建 gcov 整合 |
| **規則引擎** | 靈活的編譯規則定義 |

### 與純 Unity 比較

| 特性 | Unity | Ceedling |
|------|-------|----------|
| **Mock 支援** | 需手動撰寫 | 自動生成 |
| **專案設定** | 手動 Makefile | YAML 配置 |
| **依賴解析** | 手動處理 | 自動分析 |
| **代碼覆蓋率** | 需整合 gcov | 內建支援 |
| **學習曲線** | 低 | 中 |

---

## 安裝與設定

### 系統需求

| 需求 | 說明 |
|------|------|
| **Ruby** | >= 2.7（建議 >= 3.0） |
| **GCC** | 測試編譯器 |
| **Make** | 構建工具 |

### 安裝步驟

#### 1. 安裝 Ruby（Windows）

```powershell
# 使用 Chocolatey
choco install ruby

# 或使用 RubyInstaller
# 下載: https://rubyinstaller.org/
```

#### 2. 安裝 Ceedling

```bash
# 安裝最新穩定版
gem install ceedling

# 驗證安裝
ceedling version
```

#### 3. 更新 Ceedling

```bash
# 更新到最新版本
gem update ceedling

# 查看已安裝版本
gem list ceedling
```

### 故障排除

```bash
# 如果遇到 SSL 錯誤
gem sources -r https://rubygems.org/
gem sources -a http://rubygems.org/

# 安裝特定版本
gem install ceedling -v 0.31.0
```

---

## 專案結構

### 標準 Ceedling 專案

```
my_project/
├── src/                    # 原始碼（C 檔案）
│   ├── module1.c
│   └── module1.h
├── src/                    # 或使用 src/
├── test/                   # 測試檔案
│   ├── test_module1.c
│   └── support/            # 測試支援檔案
│       └── test_module1.h
├── build/                  # 構建輸出（自動生成）
│   ├── out/               # 編譯產物
│   ├── test/out/          # 測試結果
│   └── coverage/          # 覆蓋率報告
├── vendor/unity/           # Unity 框架（子模組）
├── project.yml             # Ceedling 配置
└── rakefile.rb            # Rake 任務定義
```

### M487 專案範例結構

```
M487_Ceedling/
├── src/
│   ├── i2c_driver/
│   │   ├── i2c_driver.c
│   │   └── i2c_driver.h
│   ├── usb_msc/
│   │   ├── usb_msc.c
│   │   └── usb_msc.h
│   └── hid_parser/
│       ├── hid_parser.c
│       └── hid_parser.h
├── test/
│   ├── test_i2c_driver.c
│   ├── test_usb_msc.c
│   └── test_hid_parser.c
├── BSP/                    # Board Support Package
│   ├── M487_HAL.c
│   └── M487_HAL.h
├── project.yml
└── Gemfile
```

---

## 設定檔案

### project.yml 範例（M487）

```yaml
# project.yml - M487 Firmware Test Configuration
:project:
  :use_test_cache: YES
  :build_tests: YES
  :coverage: YES
  :coverage_tests: YES

:paths:
  :test:
    - +:test/**
    - -:test/support
  :source:
    - src/**
    - BSP/**
  :include:
    - inc/**
    - BSP/**
  :support:
    - test/support
  :unity:
    - vendor/unity/src
  :cmock:
    - vendor/cmock/src

:defines:
  :common:
    - TEST
    - M487_FAMILY
  :test:
    - -:defines/**
  :test_preprocess:
    - TEST

:files:
  :assembly:
    - src/**/*.asm

:flags:
  :test:
    :compile:
      - -fprofile-arcs
      - -ftest-coverage
      - -fprofile-arcs
      - -ftest-coverage

:cmock:
  :mock_prefix: mock_
  :when_no_prototypes: :warn
  :enforce_strict_ordering: TRUE
  :cmock_o
```

### 詳細配置說明

```yaml
# 專案設定
:project:
  :use_test_cache: YES      # 啟用測試快取（加速）
  :build_tests: YES         # 建置測試
  :coverage: YES            # 啟用覆蓋率
  :verbosity: 2             # 日誌詳細程度

# 路徑設定
:paths:
  :test:
    - +:test/**             # 包含模式
    - -:test/temp/**        # 排除模式
  :source:
    - src/**

# 巨集定義
:defines:
  :common:                  # 所有目標的定義
    - TEST
  :test:                    # 測試專用定義
    - CI_PLATFORM
  :test_preprocess:
    - DEBUG_MODE

# CMock 設定
:cmock:
  :mock_prefix: mock_        # Mock 檔案前綴
  :unity_helper: YES        # 使用 unity_helper
  :treat_externs: :error    # 未宣告的 extern 視為錯誤
```

---

## Mock 與 Stub

### CMock 如何工作

```
┌─────────────────┐
│  i2c_driver.h   │  ← 原始 Header
└────────┬────────┘
         │
         ▼ (cecdling會自動處理)
┌─────────────────┐
│ mock_i2c_driver.h │  ← 自動生成的 Mock Header
└─────────────────┘
```

### 測試範例

```c
// test_i2c_driver.c
#include "unity.h"
#include "i2c_driver.h"
#include "mock_i2c_driver.h"  // 自動生成的 Mock

void setUp(void) {
    Mock_I2C_Init();
}

void tearDown(void) {
    // Verify all expected calls were made
}

void test_I2C_WriteReg_ShouldCallHALWrite(void) {
    // Arrange: 設定 Mock 預期
    uint8_t device = 0x50;
    uint8_t reg = 0x10;
    uint8_t data = 0xAB;

    // 預期 HAL_I2C_Write 被調用
    HAL_I2C_Write_Expect(device, reg, data);
    HAL_I2C_Write_IgnoreReturnValue();

    // Act
    int32_t result = I2C_WriteReg(device, reg, data);

    // Assert
    TEST_ASSERT_EQUAL_INT32(I2C_OK, result);
}
```

### Mock 函式類型

| 巨集 | 用途 |
|------|------|
| `_Expect()` | 預期函式被調用 |
| `_ExpectAndReturn()` | 預期調用並返回值 |
| `_IgnoreReturnValue()` | 忽略返回值 |
| `_ReturnThruPtr()` | 通過指標返回值 |
| `_Ignore()` | 完全忽略此函式 |
| `_StopIgnore()` | 停止忽略 |

### 進階 Mock 用法

```c
// 多個相同函式調用
I2C_Write_Expect(0x50, 0x00, 0x11);
I2C_Write_Expect(0x50, 0x01, 0x22);
I2C_Write_Expect(0x50, 0x02, 0x33);

// 參數匹配
I2C_Write_ExpectWithArray(0x50, 0x00, data_array, 3, 3);

// 通過指標返回值
uint8_t out_data;
HAL_I2C_Read_ExpectAndReturn(0x50, 0x10);
HAL_I2C_Read_ReturnThruPtr_data(&out_data);
out_data = 0xCD;

// 多次調用
I2C_Busy_IgnoreReturn(0);  // 第一次返回 0
I2C_Busy_IgnoreReturn(0);  // 第二次返回 0
I2C_Busy_IgnoreReturn(1);  // 第三次返回 1
```

---

## M487 整合

### 設定 M487 HAL Mock

```c
// BSP/mock_M487_HAL.c
#include "M487_HAL.h"
#include "cmock.h"

// 追蹤函式調用
static int g_i2c_init_called = 0;
static uint32_t g_i2c_speed = 0;

void Mock_M487_I2C_Init(uint32_t speed) {
    g_i2c_init_called++;
    g_i2c_speed = speed;
}

int Mock_M487_I2C_GetInitCount(void) {
    return g_i2c_init_called;
}
```

### project.yml M487 配置

```yaml
:defines:
  :common:
    - TEST
    - M487_M480_SERIES
    - USE_M487_HAL
  :test:
    - MOCK_HAL

:cmock:
  :mock_path: test/mocks
  :treat_includes_as_files: YES
```

---

## 常見命令

| 命令 | 說明 |
|------|------|
| `ceedling new <name>` | 建立新專案 |
| `ceedling test:all` | 執行所有測試 |
| `ceedling test:<filename>` | 執行指定測試 |
| `ceedling test:utils` | 執行工具測試 |
| `ceedling clobber` | 清理所有構建產物 |
| `ceedling clean` | 清理測試產物 |
| `ceedling coverage` | 生成覆蓋率報告 |
| `ceedling dump` | 導出配置（偵錯用） |
| `ceedling help` | 顯示幫助 |

### 常用選項

```bash
# 詳細輸出
ceedling test:all VERBOSE=1

# 指定測試檔案
ceedling test:tests/test_i2c_driver.c

# 排除特定測試
ceedling test:all TEST exclude:integration

# 使用快取
ceedling test:all CACHE=1
```

---

## 最佳實踐

### 1. 模組化設計

```c
// 好的設計：依賴抽象 HAL 介面
int32_t I2C_WriteReg(uint8_t addr, uint8_t reg, uint8_t data) {
    return HAL_I2C_Write(addr, reg, data);  // 可 Mock
}

// 不好的設計：直接操作暫存器
int32_t I2C_WriteReg(uint8_t addr, uint8_t reg, uint8_t data) {
    I2C0->DATA = data;  // 難以 Mock
    // ...
}
```

### 2. 使用 CException 處理異常

```c
// module.h
#include "CException.h"

#define ERR_TIMEOUT  1
#define ERR_NACK      2

void USB_Transfer(uint8_t *data, uint16_t len);

// module.c
#include "module.h"

void USB_Transfer(uint8_t *data, uint16_t len) {
    if (USB_IsTimeout()) {
        Throw(ERR_TIMEOUT);
    }
    // ...
}

// test_module.c
#include "CException.h"

void test_USB_Transfer_ShouldThrow_OnTimeout(void) {
    CEXCEPTION_T err;

    Mock_USB_IsTimeout_ExpectAndReturn(1);

    Try {
        USB_Transfer(data, 100);
        TEST_FAIL_MESSAGE("Expected exception");
    } Catch(err) {
        TEST_ASSERT_EQUAL_INT(ERR_TIMEOUT, err);
    }
}
```

### 3. 組織測試群組

```c
// 使用 TEST_GROUP() 分組
TEST_GROUP(I2C_Basic);
TEST_GROUP(I2C_ErrorHandling);
TEST_GROUP(I2C_Performance);

void test_I2C_Basic_WriteRead(void) { }
void test_I2C_ErrorHandling_NackRetry(void) { }
```

---

## 延伸閱讀

- [Ceedling 官方文檔](https://github.com/ThrowTheSwitch/ceedling)
- [CMock 文檔](https://github.com/ThrowTheSwitch/CMock)
- [CException 文檔](https://github.com/ThrowTheSwitch/CException)
- [整合指南](./Integration_Guide.md)

---

**下一步**: 查看 [M487 Ceedling 範例](./M487_Examples/) 或 [整合指南](./Integration_Guide.md)

---
