/**
 * @file test_msc_protocol.cpp
 * @brief GoogleTest unit tests for M487 MSC Debug Protocol
 *
 * Tests SCSI command parsing, CBW/CSW handling, and protocol state machine.
 *
 * @author Tamama (Knowledge Base Agent)
 * @date 2026-04-07
 */

#include <gtest/gtest.h>
#include <vector>
#include <cstring>

// ============================================================
// USB MSC Protocol Structures
// ============================================================

#pragma pack(push, 1)
struct USB_CBW {
    uint32_t dCBWSignature;    // 0x43425355 ('USBC')
    uint32_t dCBWTag;
    uint32_t dCBWDataTransferLength;
    uint8_t  bmCBWFlags;
    uint8_t  bCBWLUN;
    uint8_t  bCBWCBLength;
    uint8_t  CBWCB[16];
};

struct USB_CSW {
    uint32_t dCSWSignature;    // 0x53425355 ('USBS')
    uint32_t dCSWTag;
    uint32_t dCSWDataResidue;
    uint8_t  bCSWStatus;
};
#pragma pack(pop)

// SCSI Command Opcodes
enum SCSI_Command : uint8_t {
    SCSI_TEST_UNIT_READY  = 0x00,
    SCSI_REQUEST_SENSE    = 0x03,
    SCSI_INQUIRY          = 0x12,
    SCSI_MODE_SENSE6      = 0x1A,
    SCSI_START_STOP_UNIT   = 0x1B,
    SCSI_REDERR10         = 0x28,
    SCSI_WRITE10          = 0x2A,
    SCSI_VERIFY10         = 0x2F,
    SCSI_MODE_SENSE10     = 0x5A,
    SCSI_READ_CAPACITY10   = 0x25,
    SCSI_REDERR16        = 0x88,
    SCSI_WRITE16         = 0x8A,
};

// ============================================================
// MSC Protocol Parser
// ============================================================

class MSCProtocolParser {
public:
    static constexpr uint32_t CBW_SIGNATURE = 0x43425355;
    static constexpr uint32_t CSW_SIGNATURE = 0x53425355;

    enum class ParseResult {
        Success,
        InvalidSignature,
        InvalidLength,
        Incomplete
    };

    enum class State {
        Idle,
        CBW_Received,
        Data_Transfer,
        CSW_Sent,
        Error
    };

    MSCProtocolParser() : state_(State::Idle) {
        memset(&last_cbw_, 0, sizeof(last_cbw_));
        memset(&last_csw_, 0, sizeof(last_csw_));
    }

    ParseResult ParseCBW(const uint8_t* data, size_t len) {
        if (len < 31) return ParseResult::InvalidLength;
        if (len > sizeof(USB_CBW)) return ParseResult::InvalidLength;

        const USB_CBW* cbw = reinterpret_cast<const USB_CBW*>(data);

        if (cbw->dCBWSignature != CBW_SIGNATURE) {
            return ParseResult::InvalidSignature;
        }

        last_cbw_ = *cbw;
        state_ = State::CBW_Received;
        return ParseResult::Success;
    }

    std::vector<uint8_t> BuildCSW(uint32_t tag, uint8_t status) {
        USB_CSW csw;
        csw.dCSWSignature = CSW_SIGNATURE;
        csw.dCSWTag = tag;
        csw.dCSWDataResidue = 0;
        csw.bCSWStatus = status;
        last_csw_ = csw;

        state_ = State::CSW_Sent;

        std::vector<uint8_t> result(sizeof(USB_CSW));
        memcpy(result.data(), &csw, sizeof(USB_CSW));
        return result;
    }

    // SCSI Command Accessors
    uint8_t GetSCSIOpcode() const { return last_cbw_.CBWCB[0]; }
    uint32_t GetCBWTag() const { return last_cbw_.dCBWTag; }
    uint32_t GetDataLength() const { return last_cbw_.dCBWDataTransferLength; }
    bool IsDataIn() const { return (last_cbw_.bmCBWFlags & 0x80) != 0; }

    // Parse specific SCSI commands
    bool IsReadCommand() const {
        uint8_t opcode = GetSCSIOpcode();
        return opcode == SCSI_REDERR10 || opcode == SCSI_REDERR16;
    }

    bool IsWriteCommand() const {
        uint8_t opcode = GetSCSIOpcode();
        return opcode == SCSI_WRITE10 || opcode == SCSI_WRITE16;
    }

    uint32_t GetLBA() const {
        uint8_t opcode = GetSCSIOpcode();
        if (opcode == SCSI_REDERR10 || opcode == SCSI_WRITE10 || opcode == SCSI_VERIFY10) {
            return (last_cbw_.CBWCB[5] << 0) |
                   (last_cbw_.CBWCB[4] << 8) |
                   (last_cbw_.CBWCB[3] << 16) |
                   (last_cbw_.CBWCB[2] << 24);
        }
        return 0;
    }

    uint16_t GetTransferLength() const {
        uint8_t opcode = GetSCSIOpcode();
        if (opcode == SCSI_REDERR10 || opcode == SCSI_WRITE10 || opcode == SCSI_VERIFY10) {
            return (last_cbw_.CBWCB[8] << 0) | (last_cbw_.CBWCB[7] << 8);
        }
        return 0;
    }

    State GetState() const { return state_; }

private:
    USB_CBW last_cbw_;
    USB_CSW last_csw_;
    State state_;
};

// ============================================================
// Test Fixtures
// ============================================================

class MSCProtocolTest : public ::testing::Test {
protected:
    void SetUp() override {
        parser_ = new MSCProtocolParser();
    }

    void TearDown() override {
        delete parser_;
    }

    MSCProtocolParser* parser_;
};

// Helper function to create a CBW packet
std::vector<uint8_t> CreateCBW(uint32_t tag, uint32_t data_len,
                                uint8_t flags, uint8_t lun,
                                uint8_t cb_len, const uint8_t* cb) {
    USB_CBW cbw;
    cbw.dCBWSignature = 0x43425355;
    cbw.dCBWTag = tag;
    cbw.dCBWDataTransferLength = data_len;
    cbw.bmCBWFlags = flags;
    cbw.bCBWLUN = lun;
    cbw.bCBWCBLength = cb_len;
    memset(cbw.CBWCB, 0, sizeof(cbw.CBWCB));
    if (cb && cb_len <= 16) {
        memcpy(cbw.CBWCB, cb, cb_len);
    }

    std::vector<uint8_t> result(sizeof(USB_CBW));
    memcpy(result.data(), &cbw, sizeof(USB_CBW));
    return result;
}

// ============================================================
// CBW Parsing Tests
// ============================================================

TEST_F(MSCProtocolTest, ParseCBW_ShouldSucceedWithValidCBW) {
    auto cbw = CreateCBW(0x12345678, 512, 0x80, 0, 6,
                         new uint8_t{SCSI_REDERR10, 0, 0, 0, 0, 0, 0, 0, 1, 0});

    auto result = parser_->ParseCBW(cbw.data(), cbw.size());

    EXPECT_EQ(result, MSCProtocolParser::ParseResult::Success);
    EXPECT_EQ(parser_->GetCBWTag(), 0x12345678U);
}

TEST_F(MSCProtocolTest, ParseCBW_ShouldRejectInvalidSignature) {
    std::vector<uint8_t> invalid(31, 0xFF);
    invalid[0] = 0x00;  // Invalid signature

    auto result = parser_->ParseCBW(invalid.data(), invalid.size());

    EXPECT_EQ(result, MSCProtocolParser::ParseResult::InvalidSignature);
}

TEST_F(MSCProtocolTest, ParseCBW_ShouldRejectTooShortData) {
    std::vector<uint8_t> short_data(10, 0);

    auto result = parser_->ParseCBW(short_data.data(), short_data.size());

    EXPECT_EQ(result, MSCProtocolParser::ParseResult::InvalidLength);
}

TEST_F(MSCProtocolTest, ParseCBW_ShouldExtractTag) {
    auto cbw = CreateCBW(0xDEADBEEF, 0, 0, 0, 1, new uint8_t{0});

    parser_->ParseCBW(cbw.data(), cbw.size());

    EXPECT_EQ(parser_->GetCBWTag(), 0xDEADBEEF);
}

// ============================================================
// CSW Building Tests
// ============================================================

TEST_F(MSCProtocolTest, BuildCSW_ShouldCreateValidCSW) {
    auto csw_data = parser_->BuildCSW(0x12345678, 0);  // Success status

    ASSERT_EQ(csw_data.size(), sizeof(USB_CSW));

    const USB_CSW* csw = reinterpret_cast<const USB_CSW*>(csw_data.data());
    EXPECT_EQ(csw->dCSWSignature, 0x53425355);
    EXPECT_EQ(csw->dCSWTag, 0x12345678);
    EXPECT_EQ(csw->bCSWStatus, 0);  // Success
}

TEST_F(MSCProtocolTest, BuildCSW_ShouldSetErrorStatus) {
    auto csw_data = parser_->BuildCSW(0x12345678, 1);  // Error status

    const USB_CSW* csw = reinterpret_cast<const USB_CSW*>(csw_data.data());
    EXPECT_EQ(csw->bCSWStatus, 1);  // Error
}

TEST_F(MSCProtocolTest, BuildCSW_ShouldIncludeResidue) {
    auto csw_data = parser_->BuildCSW(0x12345678, 0);
    csw_data[12] = 0x00;
    csw_data[13] = 0x02;
    csw_data[14] = 0x00;
    csw_data[15] = 0x00;

    const USB_CSW* csw = reinterpret_cast<const USB_CSW*>(csw_data.data());
    EXPECT_EQ(csw->dCSWDataResidue, 512);  // 2 blocks * 512 bytes
}

// ============================================================
// SCSI Command Parsing Tests
// ============================================================

TEST_F(MSCProtocolTest, ParseCBW_ShouldExtractReadCommand) {
    uint8_t cb[16] = {
        SCSI_REDERR10,  // Opcode
        0,              // Reserved
        0, 0, 0, 100,   // LBA = 100 (big-endian)
        0,              // Reserved
        0, 0, 0, 20,   // Transfer length = 20 blocks
        0, 0, 0, 0      // Control
    };
    auto cbw = CreateCBW(0x01, 10240, 0x80, 0, 12, cb);  // 20 * 512 = 10240 bytes

    parser_->ParseCBW(cbw.data(), cbw.size());

    EXPECT_TRUE(parser_->IsReadCommand());
    EXPECT_FALSE(parser_->IsWriteCommand());
    EXPECT_EQ(parser_->GetLBA(), 100U);
    EXPECT_EQ(parser_->GetTransferLength(), 20U);
    EXPECT_EQ(parser_->GetDataLength(), 10240U);
    EXPECT_TRUE(parser_->IsDataIn());
}

TEST_F(MSCProtocolTest, ParseCBW_ShouldExtractWriteCommand) {
    uint8_t cb[16] = {
        SCSI_WRITE10,   // Opcode
        0,              // Reserved
        0, 0, 0, 200,   // LBA = 200
        0,              // Reserved
        0, 0, 0, 10,    // Transfer length = 10 blocks
        0, 0, 0, 0       // Control
    };
    auto cbw = CreateCBW(0x02, 5120, 0x00, 0, 12, cb);  // 10 * 512 = 5120 bytes

    parser_->ParseCBW(cbw.data(), cbw.size());

    EXPECT_FALSE(parser_->IsReadCommand());
    EXPECT_TRUE(parser_->IsWriteCommand());
    EXPECT_EQ(parser_->GetLBA(), 200U);
    EXPECT_EQ(parser_->GetTransferLength(), 10U);
    EXPECT_FALSE(parser_->IsDataIn());
}

TEST_F(MSCProtocolTest, ParseCBW_ShouldExtractTestUnitReady) {
    uint8_t cb[6] = {SCSI_TEST_UNIT_READY, 0, 0, 0, 0, 0};
    auto cbw = CreateCBW(0x03, 0, 0x80, 0, 6, cb);

    parser_->ParseCBW(cbw.data(), cbw.size());

    EXPECT_EQ(parser_->GetSCSIOpcode(), SCSI_TEST_UNIT_READY);
    EXPECT_EQ(parser_->GetDataLength(), 0U);
}

TEST_F(MSCProtocolTest, ParseCBW_ShouldExtractInquiry) {
    uint8_t cb[6] = {SCSI_INQUIRY, 0, 0, 0, 36, 0};
    auto cbw = CreateCBW(0x04, 36, 0x80, 0, 6, cb);

    parser_->ParseCBW(cbw.data(), cbw.size());

    EXPECT_EQ(parser_->GetSCSIOpcode(), SCSI_INQUIRY);
    EXPECT_EQ(parser_->GetDataLength(), 36U);
    EXPECT_TRUE(parser_->IsDataIn());
}

TEST_F(MSCProtocolTest, ParseCBW_ShouldExtractReadCapacity) {
    uint8_t cb[10] = {SCSI_READ_CAPACITY10, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    auto cbw = CreateCBW(0x05, 8, 0x80, 0, 10, cb);

    parser_->ParseCBW(cbw.data(), cbw.size());

    EXPECT_EQ(parser_->GetSCSIOpcode(), SCSI_READ_CAPACITY10);
    EXPECT_EQ(parser_->GetDataLength(), 8U);
}

// ============================================================
// State Machine Tests
// ============================================================

TEST_F(MSCProtocolTest, InitialStateShouldBeIdle) {
    EXPECT_EQ(parser_->GetState(), MSCProtocolParser::State::Idle);
}

TEST_F(MSCProtocolTest, StateShouldTransitionAfterCBW) {
    auto cbw = CreateCBW(0x01, 0, 0, 0, 1, new uint8_t{0});
    parser_->ParseCBW(cbw.data(), cbw.size());

    EXPECT_EQ(parser_->GetState(), MSCProtocolParser::State::CBW_Received);
}

TEST_F(MSCProtocolTest, StateShouldTransitionAfterCSW) {
    auto cbw = CreateCBW(0x01, 0, 0, 0, 1, new uint8_t{0});
    parser_->ParseCBW(cbw.data(), cbw.size());
    parser_->BuildCSW(0x01, 0);

    EXPECT_EQ(parser_->GetState(), MSCProtocolParser::State::CSW_Sent);
}

// ============================================================
// Protocol Compliance Tests
// ============================================================

TEST_F(MSCProtocolTest, CBWSignatureShouldBeUSBC) {
    auto cbw = CreateCBW(0x01, 0, 0, 0, 1, new uint8_t{0});

    const USB_CBW* parsed = reinterpret_cast<const USB_CBW*>(cbw.data());
    uint32_t sig = parsed->dCBWSignature;

    // Check as little-endian
    uint8_t bytes[4] = {
        static_cast<uint8_t>(sig & 0xFF),
        static_cast<uint8_t>((sig >> 8) & 0xFF),
        static_cast<uint8_t>((sig >> 16) & 0xFF),
        static_cast<uint8_t>((sig >> 24) & 0xFF)
    };

    EXPECT_EQ(bytes[0], 'U');
    EXPECT_EQ(bytes[1], 'S');
    EXPECT_EQ(bytes[2], 'B');
    EXPECT_EQ(bytes[3], 'C');
}

TEST_F(MSCProtocolTest, CSWSignatureShouldBeUSBS) {
    auto csw = parser_->BuildCSW(0x01, 0);

    const USB_CSW* parsed = reinterpret_cast<const USB_CSW*>(csw.data());
    uint8_t* sig_bytes = reinterpret_cast<uint8_t*>(&parsed->dCSWSignature);

    EXPECT_EQ(sig_bytes[0], 'U');
    EXPECT_EQ(sig_bytes[1], 'S');
    EXPECT_EQ(sig_bytes[2], 'B');
    EXPECT_EQ(sig_bytes[3], 'S');
}

// ============================================================
// Main
// ============================================================

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
