# Unity Framework - 嵌入式 C 單元測試框架

> **框架**: ThrowTheSwitch/Unity  
> **用途**: 輕量級 C 語言單元測試框架，專為嵌入式系統設計  
> **官方網站**: https://www.throwtheswitch.org/unity  
> **GitHub**: https://github.com/ThrowTheSwitch/Unity

---

## 📋 目錄

1. [概述](#概述)
2. [特色與優勢](#特色與優勢)
3. [快速開始](#快速開始)
4. [核心概念](#核心概念)
5. [斷言 API](#斷言-api)
6. [M487 整合](#m487-整合)
7. [常見問題](#常見問題)

---

## 概述

Unity 是一個專為嵌入式系統設計的輕量級 C 語言單元測試框架。它由 ThrowTheSwitch 團隊開發，具有以下特點：

- **純 C 實現**: 無需 C++ 編譯器
- **極小佔用**: 僅需 3 個檔案（unity.c, unity.h, unity_internals.h）
- **高度可移植**: 支援 GCC、IAR、Clang、MSVC 等編譯器
- **跨平台**: 可在 Windows、Linux、macOS 上運行
- **記憶體安全**: 可選的堆疊追蹤功能

---

## 特色與優勢

### 為什麼選擇 Unity？

| 特性 | 說明 |
|------|------|
| **零相依性** | 不需要 Ruby、Python 或其他執行環境 |
| **即編即用** | 直接納入專案，無需複雜設定 |
| **硬體無關** | 可在主機上測試嵌入式程式碼 |
| **即時反饋** | 快速編譯、快速執行 |
| **色彩輸出** | 終端機彩色測試結果 |

### 與其他框架比較

| 框架 | 語言 | 記憶體佔用 | 學習曲線 | 最適場景 |
|------|------|------------|----------|----------|
| **Unity** | C | ~10KB | 低 | 嵌入式韌體 |
| **Ceedling** | C + Ruby | ~50KB | 中 | 複雜嵌入式專案 |
| **GoogleTest** | C++ | ~100KB | 中 | 主機端應用 |
| **CppUTest** | C++ | ~50KB | 高 | C++ 專案 |

---

## 快速開始

### 安裝 Unity

#### 方法 1: Git 子模組（推薦）
```bash
cd your_project
git submodule add https://github.com/ThrowTheSwitch/Unity.git Unity
```

#### 方法 2: 下載發布版
```bash
# 下載最新 release
wget https://github.com/ThrowTheSwitch/Unity/releases/latest/download/unity.zip
unzip unity.zip -d Unity
```

#### 方法 3: vcpkg
```bash
vcpkg install unity
```

### 專案結構
```
your_project/
├── Unity/              # Unity 框架
│   ├── unity.c
│   ├── unity.h
│   └── unity_internals.h
├── src/                # 原始碼
│   ├── i2c_driver.c
│   └── i2c_driver.h
└── test/               # 測試檔案
    └── test_i2c_driver.c
```

### 編譯與執行（Linux/macOS）
```bash
# 編譯測試
gcc -IUnity -c Unity/unity.c -o unity.o
gcc -IUnity -c test/test_i2c_driver.c -o test_i2c_driver.o
gcc -Isrc -c src/i2c_driver.c -o i2c_driver.o

# 連結
gcc unity.o test_i2c_driver.o i2c_driver.o -o test_runner

# 執行
./test_runner
```

### 編譯與執行（Windows）
```powershell
# 使用 GCC (MinGW)
gcc -IUnity -c Unity/unity.c -o unity.o
gcc -IUnity -c test/test_i2c_driver.c -o test_i2c_driver.o
gcc -Isrc -c src/i2c_driver.c -o i2c_driver.o
gcc unity.o test_i2c_driver.o i2c_driver.o -o test_runner.exe
.\test_runner.exe
```

### 自動化 Makefile
```makefile
# Makefile for Unity Tests
UNITY_DIR = Unity
SRC_DIR = src
TEST_DIR = test

# Compiler settings
CC = gcc
CFLAGS = -Wall -Wextra -I$(UNITY_DIR) -I$(SRC_DIR)
TARGET = test_runner

# Source files
UNITY_SRC = $(UNITY_DIR)/unity.c
TEST_SRC = $(TEST_DIR)/test_i2c_driver.c
SRC_SRC = $(SRC_DIR)/i2c_driver.c

# Object files
OBJS = $(UNITY_SRC:.c=.o) $(TEST_SRC:.c=.o) $(SRC_SRC:.c=.o)

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
```

---

## 核心概念

### 測試檔案結構
```c
// test_i2c_driver.c
#include "unity.h"
#include "i2c_driver.h"

// Optional: setUp() runs before each test
void setUp(void) {
    // Initialize test fixtures
    // Reset global states
}

// Optional: tearDown() runs after each test
void tearDown(void) {
    // Cleanup resources
    // Verify invariants
}

// Test case 1
void test_I2C_WriteReg_ShouldReturnSuccess(void) {
    // Arrange: Prepare test data
    uint8_t device_addr = 0x50;
    uint8_t reg_addr = 0x10;
    uint8_t write_data = 0xAB;

    // Act: Call function under test
    int32_t result = I2C_WriteReg(device_addr, reg_addr, write_data);

    // Assert: Verify behavior
    TEST_ASSERT_EQUAL_INT32(I2C_OK, result);
}

// Test case 2
void test_I2C_ReadReg_ShouldHandleNack(void) {
    // Test NACK retry logic
    // ...
}

int main(void) {
    UNITY_BEGIN();

    // Run tests
    RUN_TEST(test_I2C_WriteReg_ShouldReturnSuccess);
    RUN_TEST(test_I2C_ReadReg_ShouldHandleNack);

    return UNITY_END();
}
```

### 執行流程
```
┌─────────────────┐
│   UNITY_BEGIN() │  Initialize test runner
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│   RUN_TEST()    │──▶ setUp()
         │        │    test function
         │        │    tearDown()
         │        │
         │        ▼
         │   ┌─────────────────┐
         │   │ Next test?      │
         │   └────────┬────────┘
         │            │
         ▼            │
┌─────────────────┐   │
│   UNITY_END()   │◀──┘
└─────────────────┘
```

---

## 斷言 API

### 通用斷言

| 巨集 | 說明 |
|------|------|
| `TEST_ASSERT_TRUE(condition)` | 斷言條件為真 |
| `TEST_ASSERT_FALSE(condition)` | 斷言條件為假 |
| `TEST_ASSERT_FAIL(message)` | 無條件失敗 |
| `TEST_ASSERT_MESSAGE(condition, msg)` | 自訂失敗訊息 |

### 數值比較

| 巨集 | 說明 |
|------|------|
| `TEST_ASSERT_EQUAL_INT(expected, actual)` | 比較 int |
| `TEST_ASSERT_EQUAL_UINT(expected, actual)` | 比較 unsigned int |
| `TEST_ASSERT_EQUAL_HEX(expected, actual)` | 比較十六進位 |
| `TEST_ASSERT_EQUAL_FLOAT(expected, actual)` | 比較 float |
| `TEST_ASSERT_EQUAL_DOUBLE(expected, actual)` | 比較 double |
| `TEST_ASSERT_EQUAL_INT8(expected, actual)` | 比較 int8_t |
| `TEST_ASSERT_EQUAL_INT16(expected, actual)` | 比較 int16_t |
| `TEST_ASSERT_EQUAL_INT32(expected, actual)` | 比較 int32_t |
| `TEST_ASSERT_EQUAL_INT64(expected, actual)` | 比較 int64_t |
| `TEST_ASSERT_EQUAL_UINT8(expected, actual)` | 比較 uint8_t |
| `TEST_ASSERT_EQUAL_UINT16(expected, actual)` | 比較 uint16_t |
| `TEST_ASSERT_EQUAL_UINT32(expected, actual)` | 比較 uint32_t |
| `TEST_ASSERT_EQUAL_UINT64(expected, actual)` | 比較 uint64_t |

### 記憶體比較

| 巨集 | 說明 |
|------|------|
| `TEST_ASSERT_EQUAL_MEMORY(expected, actual, len)` | 比較記憶體區塊 |
| `TEST_ASSERT_EQUAL_HEX8(expected, actual)` | 比較 uint8_t（十六進位） |
| `TEST_ASSERT_EQUAL_HEX16(expected, actual)` | 比較 uint16_t（十六進位） |
| `TEST_ASSERT_EQUAL_HEX32(expected, actual)` | 比較 uint32_t（十六進位） |

### 指標比較

| 巨集 | 說明 |
|------|------|
| `TEST_ASSERT_NULL(pointer)` | 斷言指標為 NULL |
| `TEST_ASSERT_NOT_NULL(pointer)` | 斷言指標不為 NULL |

### 字串比較

| 巨集 | 說明 |
|------|------|
| `TEST_ASSERT_EQUAL_STRING(expected, actual)` | 比較字串 |
| `TEST_ASSERT_EQUAL_STRING_LEN(expected, actual, len)` | 比較前 N 個字元 |

### 陣列比較

| 巨集 | 說明 |
|------|------|
| `TEST_ASSERT_EQUAL_INT_ARRAY(expected, actual, len)` | 比較 int 陣列 |
| `TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, actual, len)` | 比較 uint8_t 陣列 |
| `TEST_ASSERT_EQUAL_HEX_ARRAY(expected, actual, len)` | 比較十六進位陣列 |

### 位元操作

| 巨集 | 說明 |
|------|------|
| `TEST_ASSERT_BIT_HIGH(bit, value)` | 斷言指定位為 1 |
| `TEST_ASSERT_BIT_LOW(bit, value)` | 斷言指定位為 0 |
| `TEST_ASSERT_BITS_HIGH(mask, value)` | 斷言遮罩內所有位為 1 |
| `TEST_ASSERT_BITS_LOW(mask, value)` | 斷言遮罩內所有位為 0 |

### 範圍斷言

| 巨集 | 說明 |
|------|------|
| `TEST_ASSERT_INT_WITHIN(delta, expected, actual)` | 斷言數值在範圍內 |
| `TEST_ASSERT_UINT_WITHIN(delta, expected, actual)` | 斷言無符號數在範圍內 |

### 異常處理（需啟用 UNITY_INCLUDE_CONFIG_H）

| 巨集 | 說明 |
|------|------|
| `TEST_EXPECT_EXCEPTION(type, code)` | 預期拋出異常 |
| `TEST_ASSERT_EXCEPTION(type, code)` | 斷言拋出異常 |

---

## M487 整合

### 設定 M487 測試環境

#### 1. 建立測試 Mock 層
由於 M487 硬體無法在主機上直接訪問，需要建立 Mock 層：

```c
// mock_m487_hal.h
#ifndef MOCK_M487_HAL_H
#define MOCK_M487_HAL_H

#include <stdint.h>

// Mock I2C registers
extern uint32_t MOCK_I2C0_BASE;
extern uint8_t  MOCK_I2C0_STATUS;

// Mock functions for HAL
void Mock_I2C_Init(void);
void Mock_I2C_Reset(void);
void Mock_I2C_SetData(uint8_t data);
uint8_t Mock_I2C_GetData(void);

#endif // MOCK_M487_HAL_H
```

#### 2. 在編譯時切換實作
```c
// i2c_driver.c - Conditional compilation
#include "i2c_driver.h"

#ifdef TEST
    #include "mock_m487_hal.h"
    #define I2C_REG_READ(addr) Mock_I2C_RegRead(addr)
    #define I2C_REG_WRITE(addr, val) Mock_I2C_RegWrite(addr, val)
#else
    #include "M480.h"  // Real M487 headers
    #define I2C_REG_READ(addr) (*(volatile uint32_t *)(addr))
    #define I2C_REG_WRITE(addr, val) (*(volatile uint32_t *)(addr) = (val))
#endif
```

#### 3. 編譯測試
```bash
# Compile with TEST flag
gcc -DTEST -IUnity -Isrc -Itest -c src/i2c_driver.c -o i2c_driver.o
gcc -DTEST -IUnity -Isrc -Itest -c test/mock_m487_hal.c -o mock_m487_hal.o
gcc -IUnity -Itest unity.c test/test_i2c_driver.c i2c_driver.o mock_m487_hal.o -o test_runner
```

### M487 專用範例

請參考 [M487_Examples/test_i2c_driver.c](./M487_Examples/test_i2c_driver.c) 獲取完整範例。

---

## 常見問題

### Q1: 如何處理硬體相依性？
**A**: 使用條件編譯和 Mock 層。在測試時定義 `TEST` 巨集，切換到 Mock 實作。

### Q2: 如何測試中斷處理？
**A**: 使用函式指標模擬中斷觸發：
```c
void test_GPIO_Interrupt_ShouldTriggerCallback(void) {
    // Arrange
    gpio_callback_t callback_called = 0;
    GPIO_SetCallback(PA0, test_callback, &callback_called);

    // Act - simulate interrupt
    GPIO_SimulateInterrupt(PA0);

    // Assert
    TEST_ASSERT_TRUE(callback_called);
}
```

### Q3: 如何處理全域變數？
**A**: 在 `setUp()` 中初始化，`tearDown()` 中驗證：
```c
static I2C_HandleTypeDef *pCurrentHandle;

void setUp(void) {
    memset(&i2c_handle, 0, sizeof(i2c_handle));
    pCurrentHandle = &i2c_handle;
}

void tearDown(void) {
    // Verify state after test
    TEST_ASSERT_FALSE(i2c_handle.busy);
}
```

### Q4: 如何增加自訂斷言？
**A**: 使用 `TEST_ASSERT_MAX_ERROR` 或定義新巨集：
```c
#define TEST_ASSERT_I2C_OK(status) \
    TEST_ASSERT_EQUAL_INT32(I2C_OK, status)
```

---

## 延伸閱讀

- [Unity 官方文檔](https://github.com/ThrowTheSwitch/Unity)
- [Unity API 參考](https://github.com/ThrowTheSwitch/Unity/blob/master/docs/UnityAssertionsSummary.md)
- [M487_Examples 目錄](./M487_Examples/)
- [最佳實踐](./Best_Practices.md)

---

**下一步**: 查看 [M487 專用範例](./M487_Examples/) 或 [Ceedling 框架](./Ceedling/README.md)

---
