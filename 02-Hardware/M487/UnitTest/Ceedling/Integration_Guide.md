# Ceedling 整合指南

> 本指南說明如何將 Ceedling 整合到現有的 M487_ScsiTool 專案中。

---

## 📑 目錄

1. [整合前置準備](#整合前置準備)
2. [初始化 Ceedling](#初始化-ceedling)
3. [配置 project.yml](#配置-projectyml)
4. [遷移現有測試](#遷移現有測試)
5. [Mock HAL 整合](#mock-hal-整合)
6. [持續整合](#持續整合)
7. [故障排除](#故障排除)

---

## 整合前置準備

### 需求檢查

```bash
# 檢查 Ruby 版本
ruby --version
# 預期輸出: ruby 3.x.x

# 檢查 GCC 版本
gcc --version
# 預期輸出: gcc version 10.x.x 或更高

# 檢查 Make 版本
make --version
# 預期輸出: GNU Make 4.x
```

### 安裝 Ceedling

```bash
# 安裝 Ceedling
gem install ceedling

# 驗證安裝
ceedling version
# 預期輸出: Ceedling v0.31.0 或更高
```

---

## 初始化 Ceedling

### 方法 1：從頭建立（推薦新專案）

```bash
cd D:\AiWorkSpace\KM\KM-collect\M487_ScsiTool
ceedling new . --force
```

此命令會建立：
```
├── project.yml
├── src/
├── test/
├── build/
└── vendor/
```

### 方法 2：整合到現有專案

```bash
# 1. 建立必要目錄
mkdir -p test test/support vendor

# 2. 下載 Unity 和 CMock
git submodule add https://github.com/ThrowTheSwitch/Unity.git vendor/unity
git submodule add https://github.com/ThrowTheSwitch/CMock.git vendor/cmock
git submodule add https://github.com/ThrowTheSwitch/CException.git vendor/cexception

# 3. 建立 project.yml
touch project.yml
```

---

## 配置 project.yml

### 基本配置（M487）

```yaml
# project.yml for M487_ScsiTool
:project:
  :use_test_cache: YES
  :build_tests: YES
  :coverage: YES
  :coverage_tests: YES
  :verbosity: 3

:paths:
  :test:
    - +:test/**
    - -:test/support
  :source:
    - src/**
    - src/M487/**     # M487 特定程式碼
  :include:
    - inc/**
    - src/M487/**
  :support:
    - test/support
  :unity:
    - vendor/unity/src
  :cmock:
    - vendor/cmock/src

:defines:
  :common:
    - TEST
    - M487_M480_SERIES
    - __BYTE_ORDER__=1234
    - __ORDER_LITTLE_ENDIAN__=1234
  :test:
    - -:defines/**
    - UNIT_TEST

:files:
  :assembly:
    - src/**/*.asm

:cmock:
  :mock_prefix: mock_
  :when_no_prototypes: :warn
  :enforce_strict_ordering: TRUE
  :plugins:
    - ceedling
    - ignore
    - callback
  :treat_externs: :error
```

### 進階配置（代碼覆蓋率）

```yaml
:flags:
  :test:
    :compile:
      - -fprofile-arcs
      - -ftest-coverage
    :link:
      - -fprofile-arcs
      - -ftest-coverage

:gcov:
  :html_report: YES
  :html_report_details: YES
  :verbose: YES
  :exclude:
    - test/**
    - vendor/**
    - build/**
```

---

## 遷移現有測試

### 從 Unity 遷移到 Ceedling

假設你現有 Unity 測試：

```c
// test_i2c_driver.c (原有)
#include "unity.h"
#include "i2c_driver.h"
#include "mock_hal.h"  // 手動 Mock

void test_I2C_WriteReg_ShouldSucceed(void) {
    Mock_HAL_I2C_Init();
    int32_t result = I2C_WriteReg(0x50, 0x10, 0xAB);
    TEST_ASSERT_EQUAL_INT32(I2C_OK, result);
}
```

遷移到 Ceedling：

```c
// test/test_i2c_driver.c (Ceedling 格式)
#include "unity.h"
#include "unity.h"  // 確保在最前
#include "i2c_driver.h"
#include "mock_i2c_driver.h"  // CMock 自動生成

void setUp(void) {
    // Mock framework 的 setUp
}

void tearDown(void) {
    // Mock framework 的 tearDown
}

void test_I2C_WriteReg_ShouldSucceed(void) {
    // Arrange
    HAL_I2C_Write_ExpectAndReturn(0x50, 0x10, 0xAB, HAL_OK);

    // Act
    int32_t result = I2C_WriteReg(0x50, 0x10, 0xAB);

    // Assert
    TEST_ASSERT_EQUAL_INT32(I2C_OK, result);
}
```

### 檔案重新組織

```
# 原有結構
test/
├── unity/
│   ├── unity.c
│   ├── unity.h
│   └── unity_internals.h
├── mock_hal.c
├── mock_hal.h
├── test_i2c_driver.c
└── run_tests.sh

# Ceedling 結構
vendor/                    # 框架程式碼
├── unity/
├── cmock/
└── cexception/
test/                      # 測試程式碼
├── test_i2c_driver.c      # 直接在 test/ 目錄
├── test_hid_parser.c
└── support/               # 支援檔案
    └── test_specific.c
src/                       # 原始碼
    ├── i2c_driver.c
    └── i2c_driver.h
build/                     # 自動生成
```

---

## Mock HAL 整合

### 1. 建立 HAL Mock 層

```c
// test/support/M487_HAL_mock.c
#include "M487_HAL.h"
#include "cmock.h"

// 追蹤函式調用次數
static int g_i2c_init_count = 0;
static uint32_t g_i2c_configured_speed = 0;

void HAL_I2C_Init_CMockReset(void) {
    g_i2c_init_count = 0;
    g_i2c_configured_speed = 0;
}

int HAL_I2C_Init(uint32_t speed) {
    g_i2c_init_count++;
    g_i2c_configured_speed = speed;
    return HAL_OK;
}

uint32_t HAL_I2C_GetConfiguredSpeed(void) {
    return g_i2c_configured_speed;
}
```

### 2. 配置 Mock 自動生成

```yaml
# project.yml
:cmock:
  :mock_path: test/mocks
  :mock_prefix: mock_
  :helpers:
    - test/support
  :treat_includes:
    - M487_HAL.h  # Mock 這個 Header
```

### 3. 在測試中使用 Mock

```c
// test/test_usb_msc.c
#include "unity.h"
#include "unity.h"
#include "usb_msc.h"
#include "mock_M487_USB.h"
#include "mock_M487_GPIO.h"

void setUp(void) { }

void tearDown(void) { }

void test_USB_MSC_Init_ShouldConfigurePins(void) {
    // Expect GPIO configuration calls
    GPIO_SetMode_Expect(GPIO_PORT_A, 0x0F, GPIO_MODE_OUTPUT);
    GPIO_SetMode_IgnoreArg_mode();
    GPIO_SetMode_ReturnThruPtr_mode(0);

    // Expect USB clock enable
    USB_Clock_Enable_ExpectAndReturn(HAL_OK);

    // Act
    int32_t result = USB_MSC_Init();

    // Assert
    TEST_ASSERT_EQUAL_INT32(USB_OK, result);
}
```

---

## 持續整合

### GitHub Actions 整合

```yaml
# .github/workflows/unit-tests.yml
name: Unit Tests

on:
  push:
    branches: [main, develop]
  pull_request:
    branches: [main]

jobs:
  ceedling-test:
    runs-on: ubuntu-latest
    container: ruby:3.0

    steps:
      - name: Checkout code
        uses: actions/checkout@v3
        with:
          submodules: true

      - name: Install Ceedling
        run: |
          gem install ceedling

      - name: Install GCC
        run: apt-get update && apt-get install -y gcc make

      - name: Run tests
        run: |
          ceedling clobber
          ceedling test:all

      - name: Upload coverage
        if: always()
        uses: actions/upload-artifact@v3
        with:
          name: coverage-report
          path: build/coverage/**/*.html
```

### 本地 CI 腳本

```bash
#!/bin/bash
# run_tests.sh - 本地 CI 腳本

set -e

echo "=== M487_ScsiTool Unit Tests ==="
echo ""

# 安裝依賴
echo "[1/4] Checking dependencies..."
ruby --version || exit 1
gcc --version || exit 1

# 清理
echo "[2/4] Cleaning build..."
ceedling clobber

# 執行測試
echo "[3/4] Running tests..."
ceedling test:all VERBOSE=2

# 生成覆蓋率
echo "[4/4] Generating coverage..."
ceedling coverage

echo ""
echo "=== Tests Complete ==="
echo "Coverage report: build/coverage/index.html"
```

---

## 故障排除

### 常見問題

#### 1. Ruby 版本不相容

```
Error: ceedling requires Ruby version >= 2.7
```

**解決方案**：
```bash
# 使用 rbenv 安裝新版 Ruby
rbenv install 3.2.0
rbenv global 3.2.0
```

#### 2. Mock 未生成

```
Error: mock file not found: mock_i2c_driver.h
```

**解決方案**：
1. 確認 Header 有正確的函式宣告
2. 確認 CMock 路徑正確
3. 執行 `ceedling dump` 檢查配置

#### 3. 編譯錯誤

```
undefined reference to `I2C_Init'
```

**解決方案**：
- 確認 `project.yml` 中的 `:source` 路徑包含所有原始碼
- 檢查 Header 是否有 `#ifndef` 保護

#### 4. 覆蓋率為 0%

**解決方案**：
```yaml
# project.yml 確認啟用覆蓋率
:project:
  :coverage: YES
  :coverage_tests: YES

:flags:
  :test:
    :compile:
      - -fprofile-arcs
      - -ftest-coverage
```

### 偵錯技巧

```bash
# 導出完整配置
ceedling dump > config_dump.yml

# 執行單一測試（詳細輸出）
ceedling test:tests/test_i2c_driver.c VERBOSE=3

# 查看 Mock 生成日誌
ceedling test:all DEBUG=1
```

---

## 延伸資源

- [Ceedling GitHub](https://github.com/ThrowTheSwitch/ceedling)
- [CMock 文檔](https://github.com/ThrowTheSwitch/CMock)
- [Unity 官方論壇](https://www.throwtheswitch.org/)

---
