# Unity 最佳實踐指南

> 本文件收錄 Unity 單元測試框架的最佳實踐，適用於 M487_ScsiTool 嵌入式專案。

---

## 📑 目錄

1. [測試命名規範](#測試命名規範)
2. [測試結構原則](#測試結構原則)
3. [Mock 與 Stub 策略](#mock-與-stub-策略)
4. [硬體抽象層測試](#硬體抽象層測試)
5. [常見陷阱](#常見陷阱)
6. [效能優化](#效能優化)
7. [持續整合](#持續整合)

---

## 測試命名規範

### 命名格式
```
test_<Module>_<Scenario>_<ExpectedResult>
```

### 範例

| 測試名稱 | 說明 |
|----------|------|
| `test_I2C_WriteReg_ShouldReturnSuccess` | I2C 寫入應成功 |
| `test_I2C_WriteReg_ShouldRetryOnNack` | NACK 時應重試 |
| `test_HID_ParseReport_ShouldExtractReportID` | 解析報告應提取 ID |
| `test_UART_Log_ShouldTruncateLongMessage` | 長訊息應截斷 |

### 禁止的命名
```c
// ❌ 不好的命名
void test1(void) { }          // 無意義
void test_write(void) { }      // 含糊不清
void TestCase2(void) { }       // 不一致的大小寫

// ✅ 好的命名
void test_I2C_WriteReg_ShouldReturnSuccess_WhenValidAddress(void);
```

---

## 測試結構原則

### AAA 模式（Arrange-Act-Assert）

```c
void test_I2C_ReadReg_ShouldReturnCorrectData(void) {
    // ========== Arrange ==========
    // Set up test fixtures and mock expectations
    uint8_t device_addr = 0x50;
    uint8_t reg_addr = 0x10;
    uint8_t expected_data = 0xAB;

    // Configure mock to return expected value
    Mock_I2C_SetReadResponse(device_addr, reg_addr, expected_data);

    // ========== Act ==========
    // Execute the function under test
    uint8_t actual_data = I2C_ReadReg(device_addr, reg_addr);

    // ========== Assert ==========
    // Verify the behavior
    TEST_ASSERT_EQUAL_UINT8(expected_data, actual_data);
}
```

### 每個測試一個概念

```c
// ❌ 不好：在一個測試中混合多個概念
void test_I2C_AllOperations(void) {
    TEST_ASSERT_TRUE(I2C_Init());
    TEST_ASSERT_EQUAL(I2C_OK, I2C_WriteReg(0x50, 0x10, 0xAB));
    uint8_t data = I2C_ReadReg(0x50, 0x10);
    TEST_ASSERT_EQUAL(0xAB, data);
    I2C_DeInit();
}

// ✅ 好：每個測試只驗證一個概念
void test_I2C_Init_ShouldConfigurePins(void);
void test_I2C_WriteReg_ShouldReturnSuccess_OnValidInput(void);
void test_I2C_WriteReg_ShouldReturnNack_OnDeviceNotPresent(void);
void test_I2C_ReadReg_ShouldReturnCorrectData_OnValidTransaction(void);
```

### 獨立性原則

```c
// ❌ 不好：測試之間有依賴關係
void test_I2C_WriteReg_ShouldSucceed(void) {
    // Depends on previous test to init I2C
    I2C_WriteReg(0x50, 0x10, 0xAB);  // May fail if test order changes
}

// ✅ 好：每個測試完全獨立
void test_I2C_WriteReg_ShouldSucceed(void) {
    // Each test sets up its own initial state
    I2C_Init();
    int32_t result = I2C_WriteReg(0x50, 0x10, 0xAB);
    TEST_ASSERT_EQUAL_INT32(I2C_OK, result);
}
```

---

## Mock 與 Stub 策略

### Mock vs Stub

| 類型 | 用途 | 範例 |
|------|------|------|
| **Mock** | 驗證互動（被調用次數、參數） | 驗證 I2C_WriteReg 被調用 |
| **Stub** | 提供固定返回值 | I2C_ReadReg 總是返回 0xAB |

### Mock 範例（驗證互動）

```c
// test_uart_debug.c
#include "unity.h"
#include "mock_uart_hal.h"

void test_UART_Log_ShouldTransmitFormattedMessage(void) {
    // Arrange
    const char *message = "Error: File not found";
    uint8_t expected_format = UART_FMT_TIMESTAMP | UART_FMT_LEVEL;

    // Expect UART_Transmit to be called with correct parameters
    UART_Transmit_Expect(UART_DEBUG, "[ERR] Error: File not found\r\n", 28);
    UART_Transmit_IgnoreReturnValue();  // Don't care about return value

    // Act
    UART_Log(UART_LEVEL_ERROR, message);

    // Assert - handled by Expect
}
```

### Stub 範例（提供返回值）

```c
// Stub implementation for testing
uint8_t Stub_I2C_ReadReg(uint8_t addr, uint8_t reg) {
    // Return predetermined values based on inputs
    if (addr == 0x50 && reg == 0x10) {
        return 0xAB;  // Temperature sensor reading
    }
    return 0xFF;  // Default: register not found
}
```

### 分層 Mock 策略（M487 專用）

```
┌─────────────────────────────────────────┐
│           Test Layer (Unity)            │
├─────────────────────────────────────────┤
│    Mock Layer (mock_m487_hal.c)         │
│    - Mock_I2C_Transfer()                │
│    - Mock_USB_SendData()                │
│    - Mock_GPIO_ReadPin()                │
├─────────────────────────────────────────┤
│    HAL Layer (m487_hal.c)               │
│    - I2C_Master_Transfer()              │
│    - USB_EP_Send()                      │
│    - GPIO_PinRead()                     │
├─────────────────────────────────────────┤
│    Driver Layer (i2c_driver.c)          │
│    - I2C_WriteReg()                     │
│    - I2C_ReadReg()                      │
├─────────────────────────────────────────┤
│    Application Layer (hid_parser.c)     │
│    - HID_ParseReport()                  │
│    - HID_GetReport()                    │
└─────────────────────────────────────────┘
```

---

## 硬體抽象層測試

### HAL 介面定義

```c
// hal_i2c.h - HAL interface (stable)
#ifndef HAL_I2C_H
#define HAL_I2C_H

#include <stdint.h>

typedef struct {
    uint8_t  device_addr;
    uint8_t  *tx_buffer;
    uint16_t tx_length;
    uint8_t  *rx_buffer;
    uint16_t rx_length;
} HAL_I2C_Transfer_t;

/**
 * @brief Initialize I2C peripheral
 * @param bus_speed I2C bus speed in Hz
 * @return 0 on success, negative on error
 */
int32_t HAL_I2C_Init(uint32_t bus_speed);

/**
 * @brief Perform I2C master transfer
 * @param transfer Pointer to transfer configuration
 * @return Bytes transferred, negative on error
 */
int32_t HAL_I2C_Transfer(HAL_I2C_Transfer_t *transfer);

#endif // HAL_I2C_H
```

### Mock HAL 實作

```c
// mock_hal_i2c.c - Mock implementation for testing
#include "hal_i2c.h"
#include <string.h>

// Internal state for mock
static HAL_I2C_Transfer_t g_mock_transfer;
static int g_mock_transfer_count = 0;
static uint8_t g_mock_memory[256] = {0};

// Track transfer calls
void Mock_HAL_I2C_Reset(void) {
    memset(&g_mock_transfer, 0, sizeof(g_mock_transfer));
    g_mock_transfer_count = 0;
}

int32_t HAL_I2C_Init(uint32_t bus_speed) {
    // Mock always succeeds
    return 0;
}

int32_t HAL_I2C_Transfer(HAL_I2C_Transfer_t *transfer) {
    g_mock_transfer_count++;

    if (transfer == NULL) {
        return -1;  // Error: NULL pointer
    }

    // Simulate write operation
    if (transfer->tx_buffer && transfer->tx_length > 0) {
        // Store to mock memory
        uint8_t base_addr = transfer->tx_buffer[0];
        memcpy(&g_mock_memory[base_addr],
               &transfer->tx_buffer[1],
               transfer->tx_length - 1);
    }

    // Simulate read operation
    if (transfer->rx_buffer && transfer->rx_length > 0) {
        uint8_t base_addr = transfer->tx_buffer ? transfer->tx_buffer[0] : 0;
        memcpy(transfer->rx_buffer,
               &g_mock_memory[base_addr],
               transfer->rx_length);
    }

    return transfer->tx_length + transfer->rx_length;
}

// Accessors for verification
int Mock_HAL_I2C_GetTransferCount(void) {
    return g_mock_transfer_count;
}

void Mock_HAL_I2C_SetMemory(uint8_t addr, uint8_t value) {
    g_mock_memory[addr] = value;
}

uint8_t Mock_HAL_I2C_GetMemory(uint8_t addr) {
    return g_mock_memory[addr];
}
```

### 使用 Mock HAL 測試

```c
// test_i2c_driver_with_mock.c
#include "unity.h"
#include "hal_i2c.h"

extern int Mock_HAL_I2C_GetTransferCount(void);
extern void Mock_HAL_I2C_Reset(void);
extern void Mock_HAL_I2C_SetMemory(uint8_t addr, uint8_t value);

void setUp(void) {
    Mock_HAL_I2C_Reset();
    HAL_I2C_Init(400000);  // 400kHz
}

void tearDown(void) {
    // Verify mock was called expected number of times
}

void test_I2C_WriteReg_ShouldWriteToCorrectAddress(void) {
    // Arrange
    uint8_t device_addr = 0x50;
    uint8_t reg_addr = 0x10;
    uint8_t write_data = 0xAB;

    // Act
    int32_t result = I2C_WriteReg(device_addr, reg_addr, write_data);

    // Assert
    TEST_ASSERT_EQUAL_INT32(0, result);
    TEST_ASSERT_EQUAL_INT(1, Mock_HAL_I2C_GetTransferCount());
    TEST_ASSERT_EQUAL_UINT8(0xAB, Mock_HAL_I2C_GetMemory(reg_addr));
}
```

---

## 常見陷阱

### 1. 忘記重置全域狀態

```c
// ❌ 陷阱：全域狀態殘留
static uint8_t g_i2c_busy = 0;  // Global state

void test_First(void) {
    g_i2c_busy = 1;
    // ... assertions
}

void test_Second(void) {
    // g_i2c_busy is still 1 from previous test!
    // This may cause test_Second to fail unexpectedly
}

// ✅ 解決方案：在 setUp() 重置
void setUp(void) {
    g_i2c_busy = 0;  // Reset each test
}
```

### 2. 硬編碼路徑相依性

```c
// ❌ 陷阱：硬編碼路徑
FILE *log_file = fopen("C:\\temp\\debug.log", "w");

// ✅ 解決方案：使用可配置路徑
void setUp(void) {
    g_config.log_path = get_test_temp_path();
}
```

### 3. 未處理記憶體洩漏

```c
// ❌ 陷阱：分配但未釋放
void test_BufferInit_ShouldSucceed(void) {
    Buffer_t *buf = Buffer_Create(100);
    TEST_ASSERT_NOT_NULL(buf);
    // Missing: Buffer_Destroy(buf);
}

// ✅ 解決方案：在 tearDown() 清理
void tearDown(void) {
    if (buf) {
        Buffer_Destroy(buf);
    }
}
```

### 4. 浮點數精確度問題

```c
// ❌ 陷阱：直接比較浮點數
float result = calculate_pi();
TEST_ASSERT_EQUAL_FLOAT(3.14159, result);  // May fail due to precision

// ✅ 解決方案：使用範圍斷言
TEST_ASSERT_FLOAT_WITHIN(0.0001, 3.14159, result);
```

---

## 效能優化

### 減少編譯時間

```makefile
# Parallel compilation
.PHONY: all test clean

all: test_runner

# Use pattern rules for parallel compilation
%.o: %.c | unity.o
	$(CC) $(CFLAGS) -c $< -o $@

# Run tests in parallel if multiple test files
test: test_runner
	./test_runner
```

### 快速反饋迴圈

```bash
# Run only changed tests
./test_runner --run_test=test_*  # Run specific tests
./test_runner --gtest_filter="*I2C*"  # Filter tests
```

### 增量測試

```makefile
# Only rebuild changed files
test_runner: $(OBJS)
	$(CC) $(OBJS) -o $@

%.o: %.c
# Only recompile if source or Unity.h changed
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(OBJS:.o=.d)
```

---

## 持續整合

### GitHub Actions 範例

```yaml
# .github/workflows/unit-test.yml
name: Unit Tests

on:
  push:
    branches: [main, develop]
  pull_request:
    branches: [main]

jobs:
  test:
