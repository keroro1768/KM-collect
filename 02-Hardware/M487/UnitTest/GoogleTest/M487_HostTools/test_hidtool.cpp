/**
 * @file test_hidtool.cpp
 * @brief GoogleTest unit tests for M487 HID tool (hidtool.py C++ wrapper)
 *
 * Tests HID device detection, report sending/receiving, and error handling.
 *
 * @author Tamama (Knowledge Base Agent)
 * @date 2026-04-07
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <vector>
#include <cstring>

// ============================================================
// Mock HID API Layer
// ============================================================

class MockHIDAPI {
public:
    static MockHIDAPI& Instance() {
        static MockHIDAPI instance;
        return instance;
    }

    void Reset() {
        connected_devices_.clear();
        last_write_data_.clear();
        write_call_count_ = 0;
        read_error_code_ = 0;
        write_error_code_ = 0;
        pending_input_.clear();
    }

    void AddDevice(uint16_t vid, uint16_t pid, const std::string& serial = "") {
        DeviceInfo dev;
        dev.vid = vid;
        dev.pid = pid;
        dev.serial = serial;
        dev.connected = false;
        connected_devices_.push_back(dev);
    }

    void ConnectDevice(int index) {
        if (index >= 0 && index < static_cast<int>(connected_devices_.size())) {
            connected_devices_[index].connected = true;
        }
    }

    void InjectInputReport(const uint8_t* data, size_t len) {
        pending_input_.assign(data, data + len);
    }

    void SetReadError(int error) { read_error_code_ = error; }
    void SetWriteError(int error) { write_error_code_ = error; }

    int Write(const std::vector<uint8_t>& data) {
        if (write_error_code_ != 0) return write_error_code_;
        last_write_data_ = data;
        write_call_count_++;
        return static_cast<int>(data.size());
    }

    int Read(std::vector<uint8_t>& buffer, size_t max_len) {
        if (read_error_code_ != 0) return read_error_code_;
        if (pending_input_.empty()) return 0;

        size_t to_copy = std::min(max_len, pending_input_.size());
        buffer.assign(pending_input_.begin(), pending_input_.begin() + to_copy);
        pending_input_.clear();
        return static_cast<int>(to_copy);
    }

    std::vector<uint8_t> GetLastWriteData() const { return last_write_data_; }
    int GetWriteCallCount() const { return write_call_count_; }
    bool IsDeviceConnected(int index) const {
        if (index >= 0 && index < static_cast<int>(connected_devices_.size())) {
            return connected_devices_[index].connected;
        }
        return false;
    }

private:
    MockHIDAPI() : write_call_count_(0), read_error_code_(0), write_error_code_(0) {}

    struct DeviceInfo {
        uint16_t vid;
        uint16_t pid;
        std::string serial;
        bool connected;
    };

    std::vector<DeviceInfo> connected_devices_;
    std::vector<uint8_t> last_write_data_;
    std::vector<uint8_t> pending_input_;
    int write_call_count_;
    int read_error_code_;
    int write_error_code_;
};

// ============================================================
// Device Under Test (Simple HID Tool Wrapper)
// ============================================================

class HIDTool {
public:
    static constexpr uint16_t M487_VID = 0x0416;
    static constexpr uint16_t M487_PID = 0x5020;

    HIDTool() : connected_(false), device_index_(-1) {}

    bool Connect(uint16_t vid, uint16_t pid, const std::string& serial = "") {
        MockHIDAPI& mock = MockHIDAPI::Instance();

        // Find matching device
        for (size_t i = 0; i < mock.GetWriteCallCount() + 10; i++) {
            if (mock.IsDeviceConnected(i)) {
                connected_ = true;
                device_index_ = static_cast<int>(i);
                return true;
            }
        }

        return false;
    }

    void Disconnect() {
        connected_ = false;
        device_index_ = -1;
    }

    bool IsConnected() const { return connected_; }

    int SendReport(const std::vector<uint8_t>& report) {
        if (!connected_) return -1;
        return MockHIDAPI::Instance().Write(report);
    }

    int ReceiveReport(std::vector<uint8_t>& buffer, size_t max_len) {
        if (!connected_) return -1;
        return MockHIDAPI::Instance().Read(buffer, max_len);
    }

private:
    bool connected_;
    int device_index_;
};

// ============================================================
// Test Fixtures
// ============================================================

class HIDToolTest : public ::testing::Test {
protected:
    void SetUp() override {
        MockHIDAPI::Instance().Reset();
        tool_ = new HIDTool();
    }

    void TearDown() override {
        delete tool_;
        MockHIDAPI::Instance().Reset();
    }

    HIDTool* tool_;
};

class HIDToolConnectedTest : public HIDToolTest {
protected:
    void SetUp() override {
        HIDToolTest::SetUp();
        // Add and connect a mock M487 device
        MockHIDAPI::Instance().AddDevice(
            HIDTool::M487_VID, HIDTool::M487_PID, "M487-001");
        MockHIDAPI::Instance().ConnectDevice(0);
    }
};

// ============================================================
// Connection Tests
// ============================================================

TEST_F(HIDToolTest, ConnectShouldSucceedWithValidVIDPID) {
    MockHIDAPI::Instance().AddDevice(
        HIDTool::M487_VID, HIDTool::M487_PID, "M487-001");
    MockHIDAPI::Instance().ConnectDevice(0);

    bool result = tool_->Connect(HIDTool::M487_VID, HIDTool::M487_PID);

    EXPECT_TRUE(result);
    EXPECT_TRUE(tool_->IsConnected());
}

TEST_F(HIDToolTest, ConnectShouldFailWithInvalidVID) {
    MockHIDAPI::Instance().AddDevice(
        HIDTool::M487_VID, HIDTool::M487_PID, "M487-001");
    MockHIDAPI::Instance().ConnectDevice(0);

    bool result = tool_->Connect(0xFFFF, HIDTool::M487_PID);

    EXPECT_FALSE(result);
    EXPECT_FALSE(tool_->IsConnected());
}

TEST_F(HIDToolTest, DisconnectShouldUpdateState) {
    MockHIDAPI::Instance().AddDevice(
        HIDTool::M487_VID, HIDTool::M487_PID, "M487-001");
    MockHIDAPI::Instance().ConnectDevice(0);

    tool_->Connect(HIDTool::M487_VID, HIDTool::M487_PID);
    ASSERT_TRUE(tool_->IsConnected());

    tool_->Disconnect();

    EXPECT_FALSE(tool_->IsConnected());
}

TEST_F(HIDToolTest, ConnectShouldFailWhenNoDevice) {
    bool result = tool_->Connect(HIDTool::M487_VID, HIDTool::M487_PID);

    EXPECT_FALSE(result);
    EXPECT_FALSE(tool_->IsConnected());
}

// ============================================================
// Report Sending Tests
// ============================================================

TEST_F(HIDToolConnectedTest, SendReportShouldReturnSuccess) {
    std::vector<uint8_t> report = {0x01, 0x02, 0x03, 0x04, 0x05};

    int result = tool_->SendReport(report);

    EXPECT_GT(result, 0);
    EXPECT_EQ(MockHIDAPI::Instance().GetWriteCallCount(), 1);

    std::vector<uint8_t> sent = MockHIDAPI::Instance().GetLastWriteData();
    EXPECT_EQ(sent, report);
}

TEST_F(HIDToolConnectedTest, SendReportShouldIncludeReportID) {
    std::vector<uint8_t> report = {0x01, 0xAB, 0xCD, 0xEF};  // Report ID: 0x01

    tool_->SendReport(report);

    std::vector<uint8_t> sent = MockHIDAPI::Instance().GetLastWriteData();
    EXPECT_EQ(sent[0], 0x01);  // First byte is Report ID
}

TEST_F(HIDToolTest, SendReportShouldFailWhenNotConnected) {
    std::vector<uint8_t> report = {0x01, 0x02, 0x03};

    int result = tool_->SendReport(report);

    EXPECT_EQ(result, -1);
}

TEST_F(HIDToolConnectedTest, SendReportShouldHandleEmptyReport) {
    std::vector<uint8_t> report;

    int result = tool_->SendReport(report);

    EXPECT_GT(result, 0);
}

// ============================================================
// Report Receiving Tests
// ============================================================

TEST_F(HIDToolConnectedTest, ReceiveReportShouldReturnData) {
    uint8_t input_data[] = {0x02, 0x11, 0x22, 0x33};
    MockHIDAPI::Instance().InjectInputReport(input_data, sizeof(input_data));

    std::vector<uint8_t> buffer(64, 0);
    int bytes_read = tool_->ReceiveReport(buffer, buffer.size());

    EXPECT_EQ(bytes_read, 4);
    EXPECT_EQ(buffer[0], 0x02);
    EXPECT_EQ(buffer[1], 0x11);
}

TEST_F(HIDToolConnectedTest, ReceiveReportShouldReturnZeroWhenNoData) {
    std::vector<uint8_t> buffer(64, 0);
    int bytes_read = tool_->ReceiveReport(buffer, buffer.size());

    EXPECT_EQ(bytes_read, 0);
}

TEST_F(HIDToolTest, ReceiveReportShouldFailWhenNotConnected) {
    std::vector<uint8_t> buffer(64, 0);
    int bytes_read = tool_->ReceiveReport(buffer, buffer.size());

    EXPECT_EQ(bytes_read, -1);
}

TEST_F(HIDToolConnectedTest, ReceiveReportShouldTruncateToMaxLen) {
    uint8_t input_data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    MockHIDAPI::Instance().InjectInputReport(input_data, sizeof(input_data));

    std::vector<uint8_t> buffer(4, 0);
    int bytes_read = tool_->ReceiveReport(buffer, 4);

    EXPECT_EQ(bytes_read, 4);
}

// ============================================================
// Error Handling Tests
// ============================================================

TEST_F(HIDToolConnectedTest, SendReportShouldReturnErrorOnWriteFailure) {
    MockHIDAPI::Instance().SetWriteError(-1);

    std::vector<uint8_t> report = {0x01, 0x02};
    int result = tool_->SendReport(report);

    EXPECT_EQ(result, -1);
}

TEST_F(HIDToolConnectedTest, ReceiveReportShouldReturnErrorOnReadFailure) {
    MockHIDAPI::Instance().SetReadError(-1);

    std::vector<uint8_t> buffer(64, 0);
    int bytes_read = tool_->ReceiveReport(buffer, buffer.size());

    EXPECT_EQ(bytes_read, -1);
}

// ============================================================
// Protocol Tests
// ============================================================

TEST_F(HIDToolConnectedTest, SendVendorCommand) {
    // Vendor-specific command format: [ReportID=0xFF][Cmd][SubCmd][Data...]
    std::vector<uint8_t> cmd = {0xFF, 0x01, 0x00, 0xAB, 0xCD};

    int result = tool_->SendReport(cmd);

    EXPECT_GT(result, 0);

    std::vector<uint8_t> sent = MockHIDAPI::Instance().GetLastWriteData();
    EXPECT_EQ(sent[0], 0xFF);  // Vendor Report ID
    EXPECT_EQ(sent[1], 0x01);  // Command: Get Info
}

TEST_F(HIDToolConnectedTest, ReadDeviceInfo) {
    // Expected response format: [ReportID][Status][Data...]
    uint8_t response[] = {0xFF, 0x00, 0x01, 0x02};  // Success response
    MockHIDAPI::Instance().InjectInputReport(response, sizeof(response));

    std::vector<uint8_t> cmd = {0xFF, 0x01, 0x00};  // Get Info command
    tool_->SendReport(cmd);

    std::vector<uint8_t> buffer(64, 0);
    int bytes_read = tool_->ReceiveReport(buffer, buffer.size());

    EXPECT_EQ(bytes_read, 4);
    EXPECT_EQ(buffer[0], 0xFF);  // Response Report ID
    EXPECT_EQ(buffer[1], 0x00);  // Status: Success
}

TEST_F(HIDToolConnectedTest, MultipleCommandSequence) {
    // Send command sequence: Get Version -> Get Status -> Get Config
    std::vector<uint8_t> cmd1 = {0xFF, 0x01, 0x00};  // Get Info
    std::vector<uint8_t> cmd2 = {0xFF, 0x02, 0x00};  // Get Status
    std::vector<uint8_t> cmd3 = {0xFF, 0x03, 0x00};  // Get Config

    EXPECT_EQ(tool_->SendReport(cmd1), 3);
    EXPECT_EQ(tool_->SendReport(cmd2), 3);
    EXPECT_EQ(tool_->SendReport(cmd3), 3);

    EXPECT_EQ(MockHIDAPI::Instance().GetWriteCallCount(), 3);
}

// ============================================================
// Performance Tests
// ============================================================

TEST_F(HIDToolConnectedTest, SendReportShouldCompleteInTime) {
    std::vector<uint8_t> report(64, 0xAB);

    auto start = std::chrono::steady_clock::now();
    int result = tool_->SendReport(report);
    auto end = std::chrono::steady_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    EXPECT_GT(result, 0);
    EXPECT_LT(duration.count(), 1000);  // Should complete within 1ms
}

TEST_F(HIDToolConnectedTest, BatchOperationsShouldBeFast) {
    const int NUM_OPS = 100;

    auto start = std::chrono::steady_clock::now();

    for (int i = 0; i < NUM_OPS; i++) {
        std::vector<uint8_t> report = {0x01, static_cast<uint8_t>(i)};
        tool_->SendReport(report);
    }

    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    EXPECT_EQ(MockHIDAPI::Instance().GetWriteCallCount(), NUM_OPS);
    EXPECT_LT(duration.count(), 100);  // 100 ops within 100ms
}

// ============================================================
// Main
// ============================================================

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
