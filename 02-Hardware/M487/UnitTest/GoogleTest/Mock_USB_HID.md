# Mock USB HID 技術指南

> 本文件說明如何在 GoogleTest 中 Mock USB HID 通訊，進行 M487 主機工具的單元測試。

---

## 📑 目錄

1. [概述](#概述)
2. [HID 模擬架構](#hid-模擬架構)
3. [Mock HID API](#mock-hid-api)
4. [測試範例](#測試範例)
5. [HID 報告模擬](#hid-報告模擬)
6. [協定測試](#協定測試)

---

## 概述

### 為什麼需要 Mock HID？

在開發 hidtool.py、msc_debug.exe 等主機工具時，直接連接真實 USB 設備會有這些問題：

| 問題 | 說明 |
|------|------|
| **環境依賴** | 需要實體設備才能測試 |
| **速度慢** | USB 通訊延遲高 |
| **不穩定** | 設備斷開連接會導致測試失敗 |
| **覆蓋率不足** | 難以測試錯誤路徑 |

### Mock 策略

```
┌─────────────────────────────────────┐
│       Test Code (GoogleTest)        │
├─────────────────────────────────────┤
│    Mock HID Library Layer           │
│    - Fake USB connection            │
│    - Simulated responses            │
├─────────────────────────────────────┤
│    Protocol Layer (Under Test)       │
│    - HID reports parsing            │
│    - Protocol state machine         │
├─────────────────────────────────────┤
│    Device Simulation                │
│    - Simulated device responses     │
│    - Error injection                │
└─────────────────────────────────────┘
```

---

## HID 模擬架構

### 基本 Mock 類別

```cpp
// mock_hid_api.h
#pragma once

#include <cstdint>
#include <vector>
#include <functional>
#include <optional>

// Simulated HID device handle
struct FakeHIDDevice {
    uint16_t vendor_id;
    uint16_t product_id;
    std::vector<uint8_t> input_buffer;
    std::vector<uint8_t> output_buffer;
    bool connected;
};

// Mock HID API
class MockHIDAPI {
public:
    // Singleton for global access
    static MockHIDAPI& Instance();

    // Device management
    void Reset();
    void SetNextDevice(uint16_t vid, uint16_t pid);
    void ConnectDevice(const FakeHIDDevice& device);
    void DisconnectDevice();

    // Data injection
    void InjectInputReport(const uint8_t* data, size_t len);
    void SetReadCallback(std::function<void(uint8_t*, size_t)> callback);

    // Verification
    std::vector<uint8_t> GetLastOutputReport() const;
    bool WasWriteCalled() const;
    int GetWriteCallCount() const;

    // Error simulation
    void SetReadError(int error_code);
    void SetWriteError(int error_code);
    void SetDeviceTimeout(int timeout_ms);

private:
    MockHIDAPI() : write_call_count_(0), read_error_(0), write_error_(0) {}
    MockHIDAPI(const MockHIDAPI&) = delete;

    std::optional<FakeHIDDevice> device_;
    std::vector<uint8_t> last_output_;
    int write_call_count_;
    int read_error_;
    int write_error_;
    std::function<void(uint8_t*, size_t)> read_callback_;
};
```

### Mock 實作

```cpp
// mock_hid_api.cpp
#include "mock_hid_api.h"
#include <cstring>
#include <algorithm>

MockHIDAPI& MockHIDAPI::Instance() {
    static MockHIDAPI instance;
    return instance;
}

void MockHIDAPI::Reset() {
    device_.reset();
    last_output_.clear();
    write_call_count_ = 0;
    read_error_ = 0;
    write_error_ = 0;
    read_callback_ = nullptr;
}

void MockHIDAPI::ConnectDevice(const FakeHIDDevice& device) {
    device_.emplace(device);
    device_->connected = true;
}

void MockHIDAPI::InjectInputReport(const uint8_t* data, size_t len) {
    if (device_) {
        device_->input_buffer.assign(data, data + len);
    }
}

std::vector<uint8_t> MockHIDAPI::GetLastOutputReport() const {
    return last_output_;
}

bool MockHIDAPI::WasWriteCalled() const {
    return write_call_count_ > 0;
}

int MockHIDAPI::GetWriteCallCount() const {
    return write_call_count_;
}
```

---

## Mock HID API

### 對應真實 HID API

| 真實 HID API | Mock 版本 | 用途 |
|--------------|-----------|------|
| `hid_init()` | `MockHIDAPI::Reset()` | 初始化 |
| `hid_open(vid, pid)` | `ConnectDevice()` | 開啟設備 |
| `hid_write(dev, data)` | `InjectOutput()` | 寫入數據 |
| `hid_read(dev, data)` | `GetNextInput()` | 讀取數據 |
| `hid_close(dev)` | `DisconnectDevice()` | 關閉設備 |

### 測試包裝器

```cpp
// hid_wrapper_test.h
#pragma once

#include "hid_wrapper.h"
#include "mock_hid_api.h"

// Test fixture for HID wrapper tests
class HIDWrapperTest : public ::testing::Test {
protected:
    void SetUp() override {
        MockHIDAPI::Instance().Reset();
        wrapper_ = new HIDWrapper();
    }

    void TearDown() override {
        delete wrapper_;
        MockHIDAPI::Instance().Reset();
    }

    void ConnectM487Device() {
        FakeHIDDevice device{
            .vendor_id = 0x0416,
            .product_id = 0x5020,
            .input_buffer = {},
            .output_buffer = {},
            .connected = false
        };
        MockHIDAPI::Instance().ConnectDevice(device);
    }

    HIDWrapper* wrapper_;
};
```

---

## 測試範例

### 連接測試

```cpp
// test/test_hid_connection.cpp
#include <gtest/gtest.h>
#include "hid_wrapper_test.h"

TEST_F(HIDWrapperTest, ConnectShouldSucceed_WithValidVIDPID) {
    ConnectM487Device();

    bool result = wrapper_->Connect(0x041