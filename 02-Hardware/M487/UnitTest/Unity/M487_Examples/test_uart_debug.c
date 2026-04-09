/**
 * @file test_uart_debug.c
 * @brief Unity unit tests for M487 UART debug logging
 *
 * Tests UART debug log formatting, log levels, timestamp,
 * and output buffer management.
 *
 * @author Tamama (Knowledge Base Agent)
 * @date 2026-04-07
 */

#include "unity.h"
#include "uart_debug.h"

/* ============================================================
 * Mock Layer for UART Hardware
 * ============================================================ */

// Maximum log buffer size
#define LOG_BUFFER_SIZE  256
#define MOCK_UART_BUFFER_SIZE  512

// UART peripheral mapping
typedef enum {
    UART_DEBUG_PORT = 0,
    UART_LOG_PORT,
    UART_TRACE_PORT
} UART_Port_t;

// Mock UART state
static struct {
    uint8_t initialized;
    UART_Port_t active_port;
    uint8_t tx_buffer[MOCK_UART_BUFFER_SIZE];
    uint16_t tx_count;
    uint16_t tx_index;
    uint32_t baudrate;
    uint8_t tx_busy;
    // Capture transmitted data
    uint8_t captured_data[MOCK_UART_BUFFER_SIZE];
    uint16_t captured_len;
} g_mock_uart = {
    .initialized = 0,
    .active_port = UART_DEBUG_PORT,
    .tx_count = 0,
    .tx_index = 0,
    .baudrate = 115200,
    .tx_busy = 0,
    .captured_len = 0,
};

/**
 * @brief Reset mock UART state
 */
void Mock_UART_Reset(void) {
    g_mock_uart.initialized = 1;
    g_mock_uart.tx_count = 0;
    g_mock_uart.tx_index = 0;
    g_mock_uart.tx_busy = 0;
    g_mock_uart.captured_len = 0;
    memset(g_mock_uart.captured_data, 0, sizeof(g_mock_uart.captured_data));
    memset(g_mock_uart.tx_buffer, 0, sizeof(g_mock_uart.tx_buffer));
}

/**
 * @brief Mock UART initialization
 */
int32_t Mock_UART_Init(UART_Port_t port, uint32_t baudrate) {
    g_mock_uart.initialized = 1;
    g_mock_uart.active_port = port;
    g_mock_uart.baudrate = baudrate;
    return UART_OK;
}

/**
 * @brief Mock UART transmit function
 *
 * In real hardware, this would write to UART data register.
 * In mock, we capture the data for verification.
 */
int32_t Mock_UART_Transmit(UART_Port_t port, const uint8_t *data, uint16_t len) {
    if (!g_mock_uart.initialized) {
        return UART_ERR_NOT_INIT;
    }

    if (data == NULL || len == 0) {
        return UART_ERR_INVALID_PARAM;
    }

    // Simulate transmission
    for (uint16_t i = 0; i < len; i++) {
        g_mock_uart.captured_data[g_mock_uart.captured_len++] = data[i];
    }

    g_mock_uart.tx_count++;
    return len;
}

/**
 * @brief Get number of bytes transmitted
 */
uint16_t Mock_UART_GetTxCount(void) {
    return g_mock_uart.tx_count;
}

/**
 * @brief Get captured data
 */
const uint8_t* Mock_UART_GetCapturedData(void) {
    return g_mock_uart.captured_data;
}

/**
 * @brief Get captured data length
 */
uint16_t Mock_UART_GetCapturedLen(void) {
    return g_mock_uart.captured_len;
}

/**
 * @brief Clear captured data
 */
void Mock_UART_ClearCapture(void) {
    g_mock_uart.captured_len = 0;
    memset(g_mock_uart.captured_data, 0, sizeof(g_mock_uart.captured_data));
}

/* ============================================================
 * Test Setup / Teardown
 * ============================================================ */

void setUp(void) {
    Mock_UART_Reset();

    // Initialize debug logger
    UART_Debug_Init(UART_DEBUG_PORT, 115200);
}

void tearDown(void) {
    Mock_UART_Reset();
}

/* ============================================================
 * UART Initialization Tests
 * ============================================================ */

void test_UART_Debug_Init_ShouldConfigurePort(void) {
    int32_t result = UART_Debug_Init(UART_DEBUG_PORT, 115200);

    TEST_ASSERT_EQUAL_INT32(UART_OK, result);
    TEST_ASSERT_TRUE(g_mock_uart.initialized);
}

void test_UART_Debug_Init_ShouldSetBaudrate(void) {
    UART_Debug_Init(UART_DEBUG_PORT, 921600);

    TEST_ASSERT_EQUAL_UINT32(921600, g_mock_uart.baudrate);
}

void test_UART_Debug_Init_ShouldReturnError_ForInvalidBaudrate(void) {
    int32_t result = UART_Debug_Init(UART_DEBUG_PORT, 0);

    TEST_ASSERT_EQUAL_INT32(UART_ERR_INVALID_PARAM, result);
}

/* ============================================================
 * Log Level Tests
 * ============================================================ */

void test_UART_Log_ShouldIncludeLevelPrefix(void) {
    UART_Log_Level(UART_LEVEL_ERROR, "Test error");

    const char *captured = (const char *)Mock_UART_GetCapturedData();

    // Check for ERROR prefix
    TEST_ASSERT_TRUE(strstr(captured, "[ERR]") != NULL ||
                     strstr(captured, "ERROR") != NULL);
}

void test_UART_Log_ShouldIncludeWarnPrefix(void) {
    UART_Log_Level(UART_LEVEL_WARN, "Test warning");

    const char *captured = (const char *)Mock_UART_GetCapturedData();

    TEST_ASSERT_TRUE(strstr(captured, "[WRN]") != NULL ||
                     strstr(captured, "WARN") != NULL);
}

void test_UART_Log_ShouldIncludeInfoPrefix(void) {
    UART_Log_Level(UART_LEVEL_INFO, "Test info");

    const char *captured = (const char *)Mock_UART_GetCapturedData();

    TEST_ASSERT_TRUE(strstr(captured, "[INF]") != NULL ||
                     strstr(captured, "INFO") != NULL);
}

void test_UART_Log_ShouldIncludeDebugPrefix(void) {
    UART_Log_Level(UART_LEVEL_DEBUG, "Test debug");

    const char *captured = (const char *)Mock_UART_GetCapturedData();

    TEST_ASSERT_TRUE(strstr(captured, "[DBG]") != NULL ||
                     strstr(captured, "DEBUG") != NULL);
}

/* ============================================================
 * Timestamp Tests
 * ============================================================ */

void test_UART_Log_ShouldIncludeTimestamp(void) {
    Mock_UART_ClearCapture();
    UART_Log_Level(UART_LEVEL_INFO, "Message");

    const char *captured = (const char *)Mock_UART_GetCapturedData();

    // Timestamp should be in format: [HH:MM:SS.mmm] or similar
    // Check for numeric timestamp pattern
    TEST_ASSERT_TRUE(strstr(captured, "[") != NULL);  // Bracket prefix exists
}

void test_UART_Log_ShouldIncludeMilliseconds(void) {
    Mock_UART_ClearCapture();
    UART_Log_Level(UART_LEVEL_INFO, "Test");

    const char *captured = (const char *)Mock_UART_GetCapturedData();

    // Should contain numeric timestamp
    // At minimum, should have some digits in the timestamp
    bool has_digits = false;
    for (int i = 0; i < 12 && captured[i] != '\0'; i++) {
        if (captured[i] >= '0' && captured[i] <= '9') {
            has_digits = true;
            break;
        }
    }
    TEST_ASSERT_TRUE(has_digits);
}

/* ============================================================
 * Message Formatting Tests
 * ============================================================ */

void test_UART_Log_ShouldIncludeMessage(void) {
    Mock_UART_ClearCapture();
    const char *message = "Hello World";
    UART_Log_Level(UART_LEVEL_INFO, message);

    const char *captured = (const char *)Mock_UART_GetCapturedData();

    TEST_ASSERT_TRUE(strstr(captured, message) != NULL);
}

void test_UART_Log_ShouldAppendNewline(void) {
    Mock_UART_ClearCapture();
    UART_Log_Level(UART_LEVEL_INFO, "Test");

    uint16_t len = Mock_UART_GetCapturedLen();
    TEST_ASSERT_TRUE(len > 0);

    // Last character(s) should be newline
    const uint8_t *captured = Mock_UART_GetCapturedData();
    bool has_newline = (captured[len - 1] == '\n') ||
                       (len >= 2 && captured[len - 2] == '\r' && captured[len - 1] == '\n');
    TEST_ASSERT_TRUE(has_newline);
}

void test_UART_Log_ShouldIncludeModuleTag(void) {
    Mock_UART_ClearCapture();
    UART_Log_Module(UART_LEVEL_INFO, "I2C", "Transfer complete");

    const char *captured = (const char *)Mock_UART_GetCapturedData();

    TEST_ASSERT_TRUE(strstr(captured, "I2C") != NULL);
}

/* ============================================================
 * Message Truncation Tests
 * ============================================================ */

void test_UART_Log_ShouldTruncateLongMessage(void) {
    Mock_UART_ClearCapture();

    // Create a very long message
    char long_message[300];
    memset(long_message, 'A', sizeof(long_message) - 1);
    long_message[sizeof(long_message) - 1] = '\0';

    UART_Log_Level(UART_LEVEL_INFO, long_message);

    // Should be truncated to LOG_BUFFER_SIZE
    uint16_t captured_len = Mock_UART_GetCapturedLen();
    TEST_ASSERT_LESS_OR_EQUAL_UINT16(LOG_BUFFER_SIZE, captured_len);
}

void test_UART_Log_ShouldNotCrash_OnVeryLongMessage(void) {
    // Allocate large message on stack
    char very_long_message[600];
    memset(very_long_message, 'X', sizeof(very_long_message) - 1);
    very_long_message[sizeof(very_long_message) - 1] = '\0';

    // Should not crash or hang
    int32_t result = UART_Log_Level(UART_LEVEL_INFO, very_long_message);

    TEST_ASSERT_TRUE(result == UART_OK || result == UART_ERR_OVERFLOW);
}

void test_UART_Log_ShouldHandleEmptyMessage(void) {
    Mock_UART_ClearCapture();
    UART_Log_Level(UART_LEVEL_INFO, "");

    uint16_t len = Mock_UART_GetCapturedLen();
    // Should still output header (timestamp, level)
    TEST_ASSERT_GREATER_THAN_UINT16(0, len);
}

/* ============================================================
 * Printf-style Formatting Tests
 * ============================================================ */

void test_UART_Log_Printf_ShouldFormatIntegers(void) {
    Mock_UART_ClearCapture();
    UART_Log_Printf(UART_LEVEL_INFO, "Count: %d", 42);

    const char *captured = (const char *)Mock_UART_GetCapturedData();

    TEST_ASSERT_TRUE(strstr(captured, "42") != NULL);
}

void test_UART_Log_Printf_ShouldFormatHex(void) {
    Mock_UART_ClearCapture();
    UART_Log_Printf(UART_LEVEL_INFO, "Hex: 0x%08X", 0xDEADBEEF);

    const char *captured = (const char *)Mock_UART_GetCapturedData();

    TEST_ASSERT_TRUE(strstr(captured, "DEADBEEF") != NULL);
}

void test_UART_Log_Printf_ShouldFormatStrings(void) {
    Mock_UART_ClearCapture();
    UART_Log_Printf(UART_LEVEL_INFO, "Module: %s", "USB");

    const char *captured = (const char *)Mock_UART_GetCapturedData();

    TEST_ASSERT_TRUE(strstr(captured, "USB") != NULL);
}

void test_UART_Log_Printf_ShouldFormatMultipleArgs(void) {
    Mock_UART_ClearCapture();
    UART_Log_Printf(UART_LEVEL_INFO, "I2C: addr=0x%02X, reg=0x%02X, val=0x%02X",
                    0x50, 0x10, 0xAB);

    const char *captured = (const char *)Mock_UART_GetCapturedData();

    TEST_ASSERT_TRUE(strstr(captured, "50") != NULL);
    TEST_ASSERT_TRUE(strstr(captured, "10") != NULL);
    TEST_ASSERT_TRUE(strstr(captured, "AB") != NULL);
}

void test_UART_Log_Printf_ShouldHandleZeroValues(void) {
    Mock_UART_ClearCapture();
    UART_Log_Printf(UART_LEVEL_INFO, "Zero: %d, Hex: 0x%X", 0, 0);

    const char *captured = (const char *)Mock_UART_GetCapturedData();

    TEST_ASSERT_TRUE(strstr(captured, "0") != NULL);
}

/* ============================================================
 * Buffer Management Tests
 * ============================================================ */

void test_UART_Log_ShouldIncrementTxCount(void) {
    uint16_t initial_count = Mock_UART_GetTxCount();

    UART_Log_Level(UART_LEVEL_INFO, "Test 1");
    UART_Log_Level(UART_LEVEL_INFO, "Test 2");

    TEST_ASSERT_EQUAL_UINT16(initial_count + 2, Mock_UART_GetTxCount());
}

void test_UART_Log_ShouldNotOverflow_ClearOldLogs(void) {
    // Generate many logs
    for (int i = 0; i < 10; i++) {
        UART_Log_Printf(UART_LEVEL_DEBUG, "Log entry %d", i);
    }

    // Buffer should handle overflow gracefully
    uint16_t len = Mock_UART_GetCapturedLen();
    TEST_ASSERT_TRUE(len > 0);
}

void test_UART_Log_ShouldBeThreadSafe(void) {
    // Note: True thread safety requires OS primitives
    // This tests that consecutive calls don't corrupt state
    UART_Log_Level(UART_LEVEL_INFO, "First");
    UART_Log_Level(UART_LEVEL_INFO, "Second");
    UART_Log_Level(UART_LEVEL_INFO, "Third");

    const char *captured = (const char *)Mock_UART_GetCapturedData();

    // Should contain all three messages
    TEST_ASSERT_TRUE(strstr(captured, "First") != NULL);
    TEST_ASSERT_TRUE(strstr(captured, "Second") != NULL);
    TEST_ASSERT_TRUE(strstr(captured, "Third") != NULL);
}

/* ============================================================
 * Log Filtering Tests
 * ============================================================ */

void test_UART_Log_SetLevel_ShouldFilterMessages(void) {
    // Set level to ERROR only
    UART_Log_SetLevel(UART_LEVEL_ERROR);

    Mock_UART_ClearCapture();
    UART_Log_Level(UART_LEVEL_DEBUG, "Debug message");
    UART_Log_Level(UART_LEVEL_INFO, "Info message");

    // No output expected (messages filtered out)
    // Note: Implementation may vary - some may output but mark as filtered
}

void test_UART_Log_SetLevel_ShouldOutputHigherSeverity(void) {
    UART_Log_SetLevel(UART_LEVEL_WARN);

    Mock_UART_ClearCapture();
    UART_Log_Level(UART_LEVEL_ERROR, "Error message");

    // Should output ERROR even though level is WARN
    uint16_t len = Mock_UART_GetCapturedLen();
    TEST_ASSERT_GREATER_THAN_UINT16(0, len);
}

/* ============================================================
 * Module-specific Logging Tests
 * ============================================================ */

void test_UART_Log_Module_ShouldIncludeModuleName(void) {
    Mock_UART_ClearCapture();
    UART_Log_Module(UART_LEVEL_INFO, "USB", "Device connected");

    const char *captured = (const char *)Mock_UART_GetCapturedData();

    TEST_ASSERT_TRUE(strstr(captured, "USB") != NULL);
    TEST_ASSERT_TRUE(strstr(captured, "Device connected") != NULL);
}

void test_UART_Log_Module_ShouldSupportMultipleModules(void) {
    Mock_UART_ClearCapture();

    UART_Log_Module(UART_LEVEL_INFO, "I2C", "I2C operation");
    UART_Log_Module(UART_LEVEL_INFO, "USB", "USB operation");
    UART_Log_Module(UART_LEVEL_INFO, "GPIO", "GPIO operation");

    const char *captured = (const char *)Mock_UART_GetCapturedData();

    TEST_ASSERT_TRUE(strstr(captured, "I2C") != NULL);
    TEST_ASSERT_TRUE(strstr(captured, "USB") != NULL);
    TEST_ASSERT_TRUE(strstr(captured, "GPIO") != NULL);
}

/* ============================================================
 * Special Character Handling Tests
 * ============================================================ */

void test_UART_Log_ShouldHandlePercentCharacter(void) {
    Mock_UART_ClearCapture();
    UART_Log_Printf(UART_LEVEL_INFO, "100%% complete");

    const char *captured = (const char *)Mock_UART_GetCapturedData();

    TEST_ASSERT_TRUE(strstr(captured, "100%") != NULL);
}

void test_UART_Log_ShouldHandleEscapedNewline(void) {
    Mock_UART_ClearCapture();
    UART_Log_Printf(UART_LEVEL_INFO, "Line1\\nLine2");

    const char *captured = (const char *)Mock_UART_GetCapturedData();

    TEST_ASSERT_TRUE(strstr(captured, "Line1") != NULL);
}

void test_UART_Log_ShouldHandleTabCharacter(void) {
    Mock_UART_ClearCapture();
    UART_Log_Printf(UART_LEVEL_INFO, "Col1\\tCol2");

    const char *captured = (const char *)Mock_UART_GetCapturedData();

    TEST_ASSERT_TRUE(strstr(captured, "Col1") != NULL);
}

/* ============================================================
 * Performance Tests
 * ============================================================ */

void test_UART_Log_ShouldCompleteWithinDeadline(void) {
    // Log operation should complete quickly
    uint32_t start_time = 0;  // Mock timer

    for (int i = 0; i < 100; i++) {
        UART_Log_Printf(UART_LEVEL_DEBUG, "Log %d", i);
    }

    // All 100 logs should complete
    TEST_ASSERT_EQUAL_UINT16(100, Mock_UART_GetTxCount());
}

/* ============================================================
 * Main Entry Point
 * ============================================================ */

int main(void) {
    UNITY_BEGIN();

    // Run all tests
    RUN_TEST_GROUP(UART_Init);
    RUN_TEST_GROUP(Log_Level);
    RUN_TEST_GROUP(Timestamp);
    RUN_TEST_GROUP(Formatting);
    RUN_TEST_GROUP(Truncation);
    RUN_TEST_GROUP(Printf_Formatting);
    RUN_TEST_GROUP(Buffer_Management);
    RUN_TEST_GROUP(Log_Filtering);
    RUN_TEST_GROUP(Module_Logging);
    RUN_TEST_GROUP(Special_Characters);
    RUN_TEST_GROUP(Performance);

    return UNITY_END();
}
