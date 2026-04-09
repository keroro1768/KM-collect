# GoogleTest (GTest) - C++ 單元測試框架

> **框架**: GoogleTest  
> **用途**: C++ 單元測試框架，專為 Windows 主機端工具設計  
> **官方網站**: https://google.github.io/googletest/  
> **GitHub**: https://github.com/google/googletest

---

## 📋 目錄

1. [概述](#概述)
2. [特色與優勢](#特色與優勢)
3. [安裝與設定](#安裝與設定)
4. [快速開始](#快速開始)
5. [斷言 API](#斷言-api)
6. [測試Fixture](#測試fixture)
7. [Mock 框架](#mock-框架)
8. [M487 主機工具測試](#m487-主機工具測試)
9. [常見範例](#常見範例)

---

## 概述

GoogleTest 是 Google 開發的 C++ 單元測試框架，廣泛用於各種規模的 C++ 專案。對於 M487_ScsiTool 專案，GoogleTest 適合用於測試：

- **主機端工具**：hidtool.py、msc_debug.exe 等
- **Windows 桌面應用**：配置工具、韌體更新工具
- **協定棧測試**：USB 協定模擬、SCP 協定測試
- **跨平台測試**：在 Windows、Linux、macOS 上運行

---

## 特色與優勢

### 為什麼選擇 GoogleTest？

| 特性 | 說明 |
|------|------|
| **C++ 原生** | 充分利用 C++ 特性（模板、命名空間） |
| **豐富的斷言** | 比 Unity 更強大的斷言集合 |
| **參數化測試** | 支援測試資料參數化 |
| **死亡測試** | 測試程式崩潰行為 |
| **XML/JSON 輸出** | CI/CD 整合方便 |
| **Visual Studio 整合** | 原生支援 VS 開發 |

### 與 Unity 比較

| 特性 | Unity | GoogleTest |
|------|-------|------------|
| **語言** | C | C++ |
| **記憶體佔用** | ~10KB | ~100KB |
| **Mock 支援** | 需 CMock | 內建 GMock |
| **參數化測試** | 不支援 | 支援 |
| **死亡測試** | 不支援 | 支援 |
| **最適場景** | 嵌入式韌體 | 主機端工具 |

---

## 安裝與設定

### 方法 1：vcpkg（推薦 Windows）

```powershell
# 安裝 vcpkg（如果尚未安裝）
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# 安裝 GoogleTest
vcpkg install gtest:x64-windows
```

### 方法 2：CMake FetchContent

```cmake
# CMakeLists.txt
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)
```

### 方法 3：直接下載

```powershell
# 下載預編譯版本
wget https://github.com/google/googletest/releases/download/release-1.12.1/googletest-1.12.1.zip
Expand-Archive googletest.zip -DestinationPath C:\libs\gtest
```

---

## 快速開始

### 基本測試檔案

```cpp
// test_example.cpp
#include <gtest/gtest.h>

// Test case: Simple arithmetic
TEST(MathTest, Addition) {
    EXPECT_EQ(2 + 2, 4);
    EXPECT_NE(2 + 2, 5);
}

TEST(MathTest, Division) {
    EXPECT_FLOAT_EQ(10.0 / 3.0, 3.333333f);
}

// Test case with fixtures
class StringTest : public ::testing::Test {
protected:
    void SetUp() override {
        str = new std::string("Hello");
    }

    void TearDown() override {
        delete str;
    }

    std::string *str;
};

TEST_F(StringTest, Length) {
    EXPECT_EQ(str->length(), 5);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
```

### 編譯與執行

```bash
# 編譯（Linux/macOS）
g++ -std=c++17 -lgtest -lgtest_main -pthread test_example.cpp -o test_example

# 編譯（Windows with MinGW）
g++ -std=c++17 -lgtest -lgtest_main test_example.cpp -o test_example.exe

# 執行
./test_example
# 或
test_example.exe
```

### CMakeLists.txt 範例

```cmake
cmake_minimum_required(VERSION 3.14)
project(M487_HostTools)

# GoogleTest setup
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)

enable_testing()

# Test executable
add_executable(test_runner
    test/test_hidtool.cpp
    test/test_protocol.cpp
)

target_link_libraries(test_runner
    gtest
    gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_runner)
```

---

## 斷言 API

### 基本斷言

| 巨集 | 說明 |
|------|------|
| `EXPECT_TRUE(cond)` | 斷言條件為真（非致命） |
| `ASSERT_TRUE(cond)` | 斷言條件為真（致命） |
| `EXPECT_FALSE(cond)` | 斷言條件為假 |
| `ASSERT_FALSE(cond)` | 斷言條件為假（致命） |
| `EXPECT_THROW(stmt, type)` | 預期拋出異常 |
| `EXPECT_NO_THROW(stmt)` | 預期不拋出異常 |

### 數值比較

| 巨集 | 說明 |
|------|------|
| `EXPECT_EQ(val1, val2)` | 相等 |
| `EXPECT_NE(val1, val2)` | 不等 |
| `EXPECT_LT(val1, val2)` | 小於 |
| `EXPECT_LE(val1, val2)` | 小於等於 |
| `EXPECT_GT(val1, val2)` | 大於 |
| `EXPECT_GE(val1, val2)` | 大於等於 |

### 浮點數比較

| 巨集 | 說明 |
|------|------|
| `EXPECT_FLOAT_EQ(a, b)` | float 相等（ULP <= 4） |
| `EXPECT_DOUBLE_EQ(a, b)` | double 相等（ULP <= 4） |
| `EXPECT_NEAR(a, b, abs_error)` | 在絕對誤差內 |

### 字串比較

| 巨集 | 說明 |
|------|------|
| `EXPECT_STREQ(s1, s2)` | C 字串相等 |
| `EXPECT_STRNE(s1, s2)` | C 字串不等 |
| `EXPECT_STRCASEEQ(s1, s2)` | 不分大小寫相等 |
| `EXPECT_STRCASENE(s1, s2)` | 不分大小寫不等 |

### C++ 字串比較

| 巨集 | 說明 |
|------|------|
| `EXPECT_EQ(s1, s2)` | std::string 相等 |
| `EXPECT_THAT(s, StartsWith("abc"))` | 使用 GMock 匹配器 |

### ASSERT vs EXPECT

```cpp
// ASSERT_* - 失敗時立即終止當前測試
TEST(FatalTest, Example) {
    ASSERT_NE(ptr, nullptr) << "Pointer is null!";
    ptr->doSomething();  // 如果上面失敗，不會執行這裡
}

// EXPECT_* - 失敗時記錄但繼續執行
TEST(NonFatalTest, Example) {
    EXPECT_EQ(x, 5) << "x should be 5";
    EXPECT_EQ(y, 10) << "y should be 10";  // 即使上面失敗，仍會執行
    // Both failures will be reported
}
```

---

## 測試Fixture

### 基本 Fixture

```cpp
class DatabaseTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Called before each test
        db = new Database(":memory:");
        db->Open();
    }

    void TearDown() override {
        // Called after each test
        db->Close();
        delete db;
    }

    Database *db;
};

TEST_F(DatabaseTest, InsertAndQuery) {
    db->Insert("key", "value");
    std::string result = db->Query("key");
    EXPECT_EQ(result, "value");
}

TEST_F(DatabaseTest, DeleteKey) {
    db->Insert("key", "value");
    db->Delete("key");
    EXPECT_TRUE(db->Query("key").empty());
}
```

### 參數化測試

```cpp
class IsPrimeTest : public ::testing::TestWithParam<int> {
};

TEST_P(IsPrimeTest, HandlesPrimes) {
    int n = GetParam();
    EXPECT_TRUE(IsPrime(n));
}

INSTANTIATE_TEST_SUITE_P(
    PrimeValues,
    IsPrimeTest,
    ::testing::Values(2, 3, 5, 7, 11, 13, 17, 19, 23, 29)
);

INSTANTIATE_TEST_SUITE_P(
    CompositeValues,
    IsPrimeTest,
    ::testing::Values(4, 6, 8, 9, 10, 12)
);
```

### 死亡測試

```cpp
TEST(DeathTest, NullPointerCrash) {
    // 測試程式崩潰行為
    EXPECT_DEATH({
        int *ptr = nullptr;
        *ptr = 42;  // 應該崩潰
    }, "");
}
```

---

## Mock 框架

### 使用 GMock

```cpp
#include <gmock/gmock.h>

class MockUSBInterface {
public:
    MOCK_METHOD(int, SendData, (const uint8_t* data, size_t len), ());
    MOCK_METHOD(int, ReceiveData, (uint8_t* data, size_t max_len), ());
    MOCK_METHOD(bool, IsConnected, (), (const));
};

using ::testing::Return;
using ::testing::ElementsAre;

TEST(USBTransferTest, SendDataShouldSucceed) {
    MockUSBInterface mock;

    EXPECT_CALL(mock, SendData)
        .With(ElementsAre(0x01, 0x02, 0x03))
        .WillOnce(Return(3));

    USBHandler handler(&mock);
    int result = handler.SendPacket({0x01, 0x02, 0x03});

    EXPECT_EQ(result, 3);
}
```

### Mock 常用匹配器

| 匹配器 | 說明 |
|--------|------|
| `_` | 任意值 |
| `A<T>()` | 任意 T 類型 |
| `Eq(x)` | 等於 x |
| `Ge(x)`, `Gt(x)`, `Le(x)`, `Lt(x)` | 比較 |
| `ContainsRegex(str)` | 正規表達式匹配 |
| `ElementsAre(a, b, ...)` | 陣列元素 |
| `Field(&Type::member, value)` | 結構體欄位 |

---

## M487 主機工具測試

### 測試 hidtool.py 的 C++ Wrapper

```cpp
// test/test_hid_wrapper.cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "hid_wrapper.h"

// Mock USB HID library
class MockHIDLibrary {
public:
    MOCK_METHOD(int, hid_init, ());
    MOCK_METHOD(int, hid_write, (hid_device *dev, const unsigned char *data, size_t len));
    MOCK_METHOD(int, hid_read, (hid_device *dev, unsigned char *data, size_t max_len));
    MOCK_METHOD(void, hid_close, (hid_device *dev));
};

class HIDWrapperTest : public ::testing::Test {
protected:
    void SetUp() override {
        wrapper = new HIDWrapper();
    }

    void TearDown() override {
        delete wrapper;
    }

    HIDWrapper *wrapper;
};

TEST_F(HIDWrapperTest, ConnectShouldInitializeDevice) {
    EXPECT_CALL(mock_hid, hid_init()).WillOnce(Return(0));

    bool result = wrapper->Connect(0x0416, 0x5020);
    EXPECT_TRUE(result);
}

TEST_F(HIDWrapperTest, SendReportShouldReturnSuccess) {
    uint8_t report[] = {0x01, 0x02, 0x03, 0x04, 0x05};

    EXPECT_CALL(mock_hid, hid_write)
        .WillOnce(Return(sizeof(report)));

    int result = wrapper->SendReport(report, sizeof(report));
    EXPECT_EQ(result, sizeof(report));
}

TEST_F(HIDWrapperTest, ReceiveShouldExtractReportID) {
    uint8_t response[64] = {0x01, 0xAB, 0xCD, 0xEF};

    EXPECT_CALL(mock_hid, hid_read)
        .WillOnce(DoAll(
            ::testing::SetArrayArgument<1>(response, response + 64),
            Return(64)
        ));

    std::vector<uint8_t> data = wrapper->ReceiveReport(64);
    EXPECT_EQ(data[0], 0x01);  // Report ID
}
```

### 測試 MSC Debug Protocol

```cpp
// test/test_msc_debug_protocol.cpp
#include <gtest/gtest.h>
#include "msc_debug_protocol.h"

class MSCProtocolTest : public ::testing::Test {
protected:
    void SetUp() override {
        protocol = new MSCProtocol();
    }

    void TearDown() override {
        delete protocol;
    }

    MSCProtocol *protocol;
};

TEST_F(MSCProtocolTest, ParseScsiCommand) {
    uint8_t scsi_read[] = {0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00};

    auto cmd = protocol->ParseSCSICommand(scsi_read, sizeof(scsi_read));

    ASSERT_TRUE(cmd.has_value());
    EXPECT_EQ(cmd->opcode, 0x28);           // READ(10)
    EXPECT_EQ(cmd->lba, 0);
    EXPECT_EQ(cmd->transfer_length, 0x20);
}

TEST_F(MSCProtocolTest, BuildSenseResponse) {
    SenseData sense = {
        .response_code = 0x70,
        .sense_key = 0x02,  // NOT READY
        .asc = 0x04,
        .ascq = 0x01
    };

    std::vector<uint8_t> response = protocol->BuildSenseResponse(sense);

    EXPECT_GE(response.size(), 18);
    EXPECT_EQ(response[0], 0x70);
    EXPECT_EQ(response[2], 0x02);
}
```

---

## 常見範例

### 測試 USB 描述符解析

```cpp
#include <gtest/gtest.h>
#include "usb_descriptors.h"

class USBDescriptorTest : public ::testing::Test {
protected:
    USBDescriptorParser parser;
};

TEST_F(USBDescriptorTest, ParseDeviceDescriptor) {
    uint8_t device_desc[] = {
        0x12,              // bLength
        0x01,              // bDescriptorType: DEVICE
        0x00, 0x02,         // bcdUSB: 2.00
        0x00,              // bDeviceClass
        0x00,              // bDeviceSubClass
        0x00,              // bDeviceProtocol
        0x40,              // bMaxPacketSize0: 64
        0x16, 0x04,         // idVendor: 0x0416 (Winbond)
        0x20, 0x50,         // idProduct: 0x5020
        0x00, 0x01,         // bcdDevice: 1.00
        0x01,              // iManufacturer
        0x02,              // iProduct
        0x03,              // iSerialNumber
        0x01               // bNumConfigurations
    };

    auto result = parser.ParseDeviceDescriptor(device_desc);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->vendor_id, 0x0416);
    EXPECT_EQ(result->product_id, 0x5020);
    EXPECT_EQ(result->max_packet_size, 64);
}

TEST_F(USBDescriptorTest, ParseConfigurationDescriptor) {
    // Configuration descriptor followed by interface and endpoint
    uint8_t config_desc[] = {
        // Configuration descriptor
        0x09, 0x02, 0x20, 0x00, 0x01, 0x01, 0x00, 0xC0, 0x32,
        // Interface descriptor
        0x09, 0x04, 0x00, 0x00, 0x02, 0x08, 0x06, 0x50, 0x00,
        // Endpoint IN
        0x07, 0x05, 0x81, 0x02, 0x40, 0x00, 0x00,
        // Endpoint OUT
        0x07, 0x05, 0x01, 0x02, 0x40, 0x00, 0x00
    };

    auto config = parser.ParseConfigurationDescriptor(
        config_desc, sizeof(config_desc));

    ASSERT_TRUE(config.has_value());
    EXPECT_EQ(config->max_power_ma, 100);  // 0x32 * 2 = 100mA
    EXPECT_EQ(config->interface_count, 1);
    EXPECT_EQ(config->endpoints.size(), 2);
}
```

---

## 延伸閱讀

- [GoogleTest 官方文檔](https://google.github.io/googletest/)
- [GoogleTest GitHub](https://github.com/google/googletest)
- [GMock 官方文檔](https://google.github.io/googletest/docs/gmock.html)

---

**下一步**: 查看 [M487 主機工具測試](./M487_HostTools/) 或 [Mock USB HID](./Mock_USB_HID.md)

---
