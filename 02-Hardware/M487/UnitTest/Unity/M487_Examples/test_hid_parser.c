/**
 * @file test_hid_parser.c
 * @brief Unity unit tests for M487 USB HID parser
 *
 * Tests HID report descriptor parsing, report ID matching,
 * and data extraction from HID reports.
 *
 * @author Tamama (Knowledge Base Agent)
 * @date 2026-04-07
 */

#include "unity.h"
#include "hid_parser.h"

/* ============================================================
 * Mock Layer for USB HID
 * ============================================================ */

// Standard HID Report Descriptor for a typical device
// 1 report ID, 8-byte input report, 8-byte output report
static const uint8_t g_mock_hid_report_descriptor[] = {
    // Report ID 1: Keyboard
    0x05, 0x01,        // Usage Page (Generic Desktop)
    0x09, 0x06,        // Usage (Keyboard)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x01,        //   Report ID (1)
    0x05, 0x07,        //   Usage Page (Key Codes)
    0x19, 0xE0,        //   Usage Minimum (224)
    0x29, 0xE7,        //   Usage Maximum (231)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x08,        //   Report Count (8)
    0x81, 0x02,        //   Input (Data, Variable, Absolute) -- Modifier byte
    0x95, 0x01,        //   Report Count (1)
    0x75, 0x08,        //   Report Size (8)
    0x81, 0x01,        //   Input (Constant) -- Reserved byte
    0x95, 0x05,        //   Report Count (5)
    0x75, 0x01,        //   Report Size (1)
    0x05, 0x08,        //   Usage Page (LEDs)
    0x19, 0x01,        //   Usage Minimum (1)
    0x29, 0x05,        //   Usage Maximum (5)
    0x91, 0x02,        //   Output (Data, Variable, Absolute) -- LED report
    0x95, 0x01,        //   Report Count (1)
    0x75, 0x03,        //   Report Size (3)
    0x91, 0x01,        //   Output (Constant) -- LED report padding
    0x95, 0x06,        //   Report Count (6)
    0x75, 0x08,        //   Report Size (8)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x65,        //   Logical Maximum (101)
    0x05, 0x07,        //   Usage Page (Key Codes)
    0x19, 0x00,        //   Usage Minimum (0)
    0x29, 0x65,        //   Usage Maximum (101)
    0x81, 0x00,        //   Input (Data, Array) -- Key array (6 bytes)
    0xC0,              // End Collection

    // Report ID 2: Mouse
    0x05, 0x01,        // Usage Page (Generic Desktop)
    0x09, 0x02,        // Usage (Mouse)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x02,        //   Report ID (2)
    0x09, 0x01,        //   Usage (Pointer)
    0xA1, 0x00,        //   Collection (Physical)
    0x05, 0x09,        //     Usage Page (Button)
    0x19, 0x01,        //     Usage Minimum (Button 1)
    0x29, 0x03,        //     Usage Maximum (Button 3)
    0x15, 0x00,        //     Logical Minimum (0)
    0x25, 0x01,        //     Logical Maximum (1)
    0x95, 0x03,        //     Report Count (3)
    0x75, 0x01,        //     Report Size (1)
    0x81, 0x02,        //     Input (Data, Variable, Absolute) -- Button states
    0x95, 0x01,        //     Report Count (1)
    0x75, 0x05,        //     Report Size (5)
    0x81, 0x01,        //     Input (Constant) -- Padding
    0x05, 0x01,        //     Usage Page (Generic Desktop)
    0x09, 0x30,        //     Usage (X)
    0x09, 0x31,        //     Usage (Y)
    0x09, 0x38,        //     Usage (Wheel)
    0x15, 0x81,        //     Logical Minimum (-127)
    0x25, 0x7F,        //     Logical Maximum (127)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x03,        //     Report Count (3)
    0x81, 0x06,        //     Input (Data, Variable, Relative) -- X, Y, Wheel
    0xC0,              //   End Collection
    0xC0,              // End Collection
};

// Mock HID parser state
static struct {
    uint8_t initialized;
    uint8_t report_buffer[64];
    uint16_t report_buffer_len;
    HID_ReportInfo_t reports[4];  // Support up to 4 reports
    uint8_t report_count;
    uint8_t last_report_id;
} g_mock_hid = {
    .initialized = 0,
    .report_count = 0,
    .last_report_id = 0,
};

/**
 * @brief Initialize mock HID parser
 */
void Mock_HID_Init(void) {
    g_mock_hid.initialized = 1;
    g_mock_hid.report_count = 0;
    g_mock_hid.last_report_id = 0;
    g_mock_hid.report_buffer_len = 0;
    memset(g_mock_hid.reports, 0, sizeof(g_mock_hid.reports));
}

/**
 * @brief Reset mock HID parser
 */
void Mock_HID_Reset(void) {
    g_mock_hid.report_count = 0;
    g_mock_hid.last_report_id = 0;
    g_mock_hid.report_buffer_len = 0;
}

/**
 * @brief Set a raw HID report for parsing
 */
void Mock_HID_SetReport(const uint8_t *data, uint16_t len) {
    memcpy(g_mock_hid.report_buffer, data, len);
    g_mock_hid.report_buffer_len = len;
}

/**
 * @brief Get report info by ID
 */
HID_ReportInfo_t* Mock_HID_GetReportInfo(uint8_t report_id) {
    for (uint8_t i = 0; i < g_mock_hid.report_count; i++) {
        if (g_mock_hid.reports[i].report_id == report_id) {
            return &g_mock_hid.reports[i];
        }
    }
    return NULL;
}

/* ============================================================
 * Test Setup / Teardown
 * ============================================================ */

void setUp(void) {
    Mock_HID_Reset();

    // Initialize parser with mock report descriptor
    int32_t result = HID_Parser_Init(g_mock_hid_report_descriptor,
                                      sizeof(g_mock_hid_report_descriptor));
    TEST_ASSERT_EQUAL_INT32(HID_OK, result);
}

void tearDown(void) {
    Mock_HID_Reset();
}

/* ============================================================
 * Parser Initialization Tests
 * ============================================================ */

void test_HID_Parser_Init_ShouldReturnSuccess_WithValidDescriptor(void) {
    int32_t result = HID_Parser_Init(g_mock_hid_report_descriptor,
                                      sizeof(g_mock_hid_report_descriptor));

    TEST_ASSERT_EQUAL_INT32(HID_OK, result);
}

void test_HID_Parser_Init_ShouldReturnError_WithNullPointer(void) {
    int32_t result = HID_Parser_Init(NULL, 100);

    TEST_ASSERT_EQUAL_INT32(HID_ERR_INVALID_PARAM, result);
}

void test_HID_Parser_Init_ShouldReturnError_WithZeroLength(void) {
    int32_t result = HID_Parser_Init(g_mock_hid_report_descriptor, 0);

    TEST_ASSERT_EQUAL_INT32(HID_ERR_INVALID_PARAM, result);
}

void test_HID_Parser_Init_ShouldReturnError_WithTooShortDescriptor(void) {
    uint8_t short_descriptor[] = {0x05, 0x01};

    int32_t result = HID_Parser_Init(short_descriptor, sizeof(short_descriptor));

    TEST_ASSERT_EQUAL_INT32(HID_ERR_INVALID_DESCRIPTOR, result);
}

/* ============================================================
 * Report ID Extraction Tests
 * ============================================================ */

void test_HID_GetReportID_ShouldReturnCorrectID_FromKeyboardReport(void) {
    uint8_t report[] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    uint8_t report_id = HID_GetReportID(report, sizeof(report));

    TEST_ASSERT_EQUAL_UINT8(0x01, report_id);
}

void test_HID_GetReportID_ShouldReturnCorrectID_FromMouseReport(void) {
    uint8_t report[] = {0x02, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00};

    uint8_t report_id = HID_GetReportID(report, sizeof(report));

    TEST_ASSERT_EQUAL_UINT8(0x02, report_id);
}

void test_HID_GetReportID_ShouldReturnZero_WhenNoReportID(void) {
    uint8_t report[] = {0x00, 0x00, 0x00, 0x00, 0x00};  // No Report ID in descriptor

    uint8_t report_id = HID_GetReportID(report, sizeof(report));

    TEST_ASSERT_EQUAL_UINT8(0x00, report_id);
}

/* ============================================================
 * Report Descriptor Parsing Tests
 * ============================================================ */

void test_HID_Parser_ShouldDetectKeyboardReport(void) {
    HID_ReportInfo_t *info = HID_GetReportInfo(0x01);

    TEST_ASSERT_NOT_NULL(info);
    TEST_ASSERT_EQUAL_UINT8(0x01, info->report_id);
    TEST_ASSERT_EQUAL_UINT8(HID_REPORT_TYPE_INPUT, info->type);
}

void test_HID_Parser_ShouldDetectMouseReport(void) {
    HID_ReportInfo_t *info = HID_GetReportInfo(0x02);

    TEST_ASSERT_NOT_NULL(info);
    TEST_ASSERT_EQUAL_UINT8(0x02, info->report_id);
}

void test_HID_Parser_ShouldReturnNull_ForNonexistentReport(void) {
    HID_ReportInfo_t *info = HID_GetReportInfo(0xFF);

    TEST_ASSERT_NULL(info);
}

void test_HID_Parser_ShouldParseReportSize(void) {
    // Keyboard input report: 1 byte modifiers + 1 byte reserved + 6 bytes keys = 8 bytes
    HID_ReportInfo_t *info = HID_GetReportInfo(0x01);

    TEST_ASSERT_NOT_NULL(info);
    TEST_ASSERT_GREATER_THAN(0, info->report_size);
}

/* ============================================================
 * Report Parsing Tests - Keyboard
 * ============================================================ */

void test_HID_ParseKeyboard_ShouldExtractModifierKeys(void) {
    // Report format: [Report ID][Modifiers][Reserved][Key1][Key2][Key3][Key4][Key5][Key6]
    uint8_t keyboard_report[] = {
        0x01,                        // Report ID
        0x02,                        // Modifiers: Left Ctrl
        0x00,                        // Reserved
        0x04,                        // Key: 'a'
        0x00, 0x00, 0x00, 0x00, 0x00 // Remaining keys
    };

    HID_KeyboardReport_t parsed;
    int32_t result = HID_ParseKeyboard(keyboard_report, sizeof(keyboard_report), &parsed);

    TEST_ASSERT_EQUAL_INT32(HID_OK, result);
    TEST_ASSERT_TRUE(parsed.modifiers.ctrl_left);
    TEST_ASSERT_EQUAL_UINT8(0x04, parsed.keycodes[0]);
}

void test_HID_ParseKeyboard_ShouldExtractMultipleKeys(void) {
    uint8_t keyboard_report[] = {
        0x01, 0x00, 0x00,  // Report ID, Modifiers, Reserved
        0x04, 0x05, 0x06, 0x07, 0x08, 0x09  // Keys a, b, c, d, e, f
    };

    HID_KeyboardReport_t parsed;
    int32_t result = HID_ParseKeyboard(keyboard_report, sizeof(keyboard_report), &parsed);

    TEST_ASSERT_EQUAL_INT32(HID_OK, result);
    TEST_ASSERT_EQUAL_UINT8(0x04, parsed.keycodes[0]);
    TEST_ASSERT_EQUAL_UINT8(0x05, parsed.keycodes[1]);
    TEST_ASSERT_EQUAL_UINT8(0x06, parsed.keycodes[2]);
}

void test_HID_ParseKeyboard_ShouldReturnError_ForWrongReportID(void) {
    uint8_t wrong_report[] = {
        0x02, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00  // Mouse report
    };

    HID_KeyboardReport_t parsed;
    int32_t result = HID_ParseKeyboard(wrong_report, sizeof(wrong_report), &parsed);

    TEST_ASSERT_EQUAL_INT32(HID_ERR_WRONG_REPORT_ID, result);
}

void test_HID_ParseKeyboard_ShouldHandleEmptyReport(void) {
    uint8_t empty_report[] = {
        0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    HID_KeyboardReport_t parsed;
    int32_t result = HID_ParseKeyboard(empty_report, sizeof(empty_report), &parsed);

    TEST_ASSERT_EQUAL_INT32(HID_OK, result);
    TEST_ASSERT_EQUAL_UINT8(0, parsed.modifiers.value);
}

/* ============================================================
 * Report Parsing Tests - Mouse
 * ============================================================ */

void test_HID_ParseMouse_ShouldExtractButtonStates(void) {
    // Report format: [Report ID][Buttons][X][Y][Wheel]
    uint8_t mouse_report[] = {
        0x02,                        // Report ID
        0x05,                        // Buttons: Left + Right
        0x10,                        // X: +16
        0xF0,                        // Y: -16 (signed)
        0x03                         // Wheel: +3
    };

    HID_MouseReport_t parsed;
    int32_t result = HID_ParseMouse(mouse_report, sizeof(mouse_report), &parsed);

    TEST_ASSERT_EQUAL_INT32(HID_OK, result);
    TEST_ASSERT_TRUE(parsed.buttons.left);
    TEST_ASSERT_TRUE(parsed.buttons.right);
    TEST_ASSERT_FALSE(parsed.buttons.middle);
}

void test_HID_ParseMouse_ShouldExtractSignedCoordinates(void) {
    uint8_t mouse_report[] = {
        0x02, 0x00, 0xFF, 0x01, 0x00  // X=-1, Y=1
    };

    HID_MouseReport_t parsed;
    int32_t result = HID_ParseMouse(mouse_report, sizeof(mouse_report), &parsed);

    TEST_ASSERT_EQUAL_INT32(HID_OK, result);
    TEST_ASSERT_EQUAL_INT8(-1, parsed.x);
    TEST_ASSERT_EQUAL_INT8(1, parsed.y);
}

void test_HID_ParseMouse_ShouldExtractWheelData(void) {
    uint8_t mouse_report[] = {
        0x02, 0x00, 0x00, 0x00, 0x7F  // Wheel: +127
    };

    HID_MouseReport_t parsed;
    int32_t result = HID_ParseMouse(mouse_report, sizeof(mouse_report), &parsed);

    TEST_ASSERT_EQUAL_INT32(HID_OK, result);
    TEST_ASSERT_EQUAL_INT8(0x7F, parsed.wheel);
}

/* ============================================================
 * Report Validation Tests
 * ============================================================ */

void test_HID_ValidateReport_ShouldReturnTrue_ForValidReport(void) {
    uint8_t valid_report[] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    bool is_valid = HID_ValidateReport(valid_report, sizeof(valid_report));

    TEST_ASSERT_TRUE(is_valid);
}

void test_HID_ValidateReport_ShouldReturnFalse_ForTooShortReport(void) {
    uint8_t short_report[] = {0x01, 0x00};

    bool is_valid = HID_ValidateReport(short_report, sizeof(short_report));

    TEST_ASSERT_FALSE(is_valid);
}

void test_HID_ValidateReport_ShouldReturnFalse_ForInvalidReportID(void) {
    uint8_t invalid_report[] = {0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    bool is_valid = HID_ValidateReport(invalid_report, sizeof(invalid_report));

    TEST_ASSERT_FALSE(is_valid);
}

/* ============================================================
 * Report Building Tests - Output Reports
 * ============================================================ */

void test_HID_BuildKeyboardLED_ShouldSetCorrectLEDs(void) {
    HID_KeyboardLED_t leds = {
        .num_lock = 1,
        .caps_lock = 0,
        .scroll_lock = 1,
        .compose = 0,
        .kana = 0
    };

    uint8_t report[64];
    uint16_t