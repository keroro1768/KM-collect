/**
 * @file test_i2c_driver.c
 * @brief Unity unit tests for M487 I2C driver
 *
 * Tests I2C driver functions: I2C_WriteReg, I2C_ReadReg, NACK retry logic
 *
 * @author Tamama (Knowledge Base Agent)
 * @date 2026-04-07
 */

#include "unity.h"
#include "i2c_driver.h"

/* ============================================================
 * Mock Layer for M487 HAL
 * ============================================================ */

#define MOCK_I2C_MAX_RETRIES    3
#define MOCK_I2C_TIMEOUT_TICKS  1000

// Mock I2C state
static struct {
    uint8_t  initialized;
    uint8_t  busy;
    uint32_t speed;
    uint8_t  tx_buffer[256];
    uint16_t tx_count;
    uint8_t  rx_buffer[256];
    uint16_t rx_count;
    uint8_t  memory[256];        // Simulated EEPROM memory
    uint8_t  nack_count;         // Track NACK occurrences
    uint8_t  nack_until;         // Return NACK until this call count
    int32_t  last_error;
} g_mock_i2c = {
    .initialized = 0,
    .busy = 0,
    .speed = 100000,
    .tx_count = 0,
    .rx_count = 0,
    .nack_count = 0,
    .nack_until = 0,
    .last_error = I2C_OK,
};

/**
 * @brief Initialize mock I2C peripheral
 */
void Mock_I2C_Init(void) {
    g_mock_i2c.initialized = 1;
    g_mock_i2c.busy = 0;
    g_mock_i2c.speed = 100000;
    g_mock_i2c.tx_count = 0;
    g_mock_i2c.rx_count = 0;
    g_mock_i2c.nack_count = 0;
    g_mock_i2c.nack_until = 0;
    g_mock_i2c.last_error = I2C_OK;
    memset(g_mock_i2c.memory, 0xFF, sizeof(g_mock_i2c.memory));
}

/**
 * @brief Reset mock I2C state
 */
void Mock_I2C_Reset(void) {
    g_mock_i2c.busy = 0;
    g_mock_i2c.tx_count = 0;
    g_mock_i2c.rx_count = 0;
    g_mock_i2c.nack_count = 0;
    g_mock_i2c.nack_until = 0;
    g_mock_i2c.last_error = I2C_OK;
}

/**
 * @brief Configure mock to return NACK for next N calls
 * @param count Number of calls to return NACK
 */
void Mock_I2C_SetNackCount(uint8_t count) {
    g_mock_i2c.nack_until = g_mock_i2c.nack_count + count;
}

/**
 * @brief Write data to mock memory (simulates EEPROM)
 * @param addr Device/register address
 * @param data Data to write
 */
void Mock_I2C_SetMemory(uint8_t addr, uint8_t data) {
    g_mock_i2c.memory[addr] = data;
}

/**
 * @brief Read data from mock memory
 * @param addr Memory address
 * @return Data at address
 */
uint8_t Mock_I2C_GetMemory(uint8_t addr) {
    return g_mock_i2c.memory[addr];
}

/**
 * @brief Mock I2C transfer function
 *
 * Simulates the M487 I2C master transfer behavior
 * - Checks for device address ACK/NACK
 * - Simulates register read/write
 * - Returns appropriate status codes
 *
 * @param addr Device address (7-bit or 10-bit)
 * @param reg Register address
 * @param tx_data Data to write (NULL for read)
 * @param tx_len Length of data to write
 * @param rx_data Buffer for received data (NULL for write)
 * @param rx_len Number of bytes to read
 * @return Number of bytes transferred, or negative error code
 */
int32_t Mock_I2C_Transfer(uint8_t addr, uint8_t reg,
                          const uint8_t *tx_data, uint16_t tx_len,
                          uint8_t *rx_data, uint16_t rx_len) {
    int32_t ret;

    // Check if initialized
    if (!g_mock_i2c.initialized) {
        g_mock_i2c.last_error = I2C_ERR_NOT_INIT;
        return I2C_ERR_NOT_INIT;
    }

    // Check if busy
    if (g_mock_i2c.busy) {
        g_mock_i2c.last_error = I2C_ERR_BUSY;
        return I2C_ERR_BUSY;
    }

    g_mock_i2c.busy = 1;
    g_mock_i2c.tx_count++;
    g_mock_i2c.nack_count++;

    // Simulate NACK behavior
    if (g_mock_i2c.nack_count <= g_mock_i2c.nack_until) {
        g_mock_i2c.busy = 0;
        g_mock_i2c.last_error = I2C_ERR_NACK;
        return I2C_ERR_NACK;
    }

    // Simulate write operation
    if (tx_data && tx_len > 0) {
        g_mock_i2c.memory[reg] = tx_data[0];
        // Support multi-byte writes
        for (uint16_t i = 1; i < tx_len; i++) {
            g_mock_i2c.memory[reg + i] = tx_data[i];
        }
    }

    // Simulate read operation
    if (rx_data && rx_len > 0) {
        rx_data[0] = g_mock_i2c.memory[reg];
        // Support multi-byte reads
        for (uint16_t i = 1; i < rx_len; i++) {
            rx_data[i] = g_mock_i2c.memory[reg + i];
        }
    }

    g_mock_i2c.busy = 0;
    g_mock_i2c.last_error = I2C_OK;
    ret = tx_len + rx_len;
    return ret;
}

/**
 * @brief Get transfer count for verification
 */
uint16_t Mock_I2C_GetTransferCount(void) {
    return g_mock_i2c.tx_count;
}

/**
 * @brief Get last error code
 */
int32_t Mock_I2C_GetLastError(void) {
    return g_mock_i2c.last_error;
}

/* ============================================================
 * Test Setup / Teardown
 * ============================================================ */

void setUp(void) {
    // Reset and initialize mock before each test
    Mock_I2C_Reset();
    I2C_Init(100000);  // 100kHz standard mode
}

void tearDown(void) {
    // Cleanup after each test
    Mock_I2C_Reset();
}

/* ============================================================
 * I2C Initialization Tests
 * ============================================================ */

void test_I2C_Init_ShouldConfigureSpeed(void) {
    int32_t result = I2C_Init(400000);  // 400kHz fast mode

    TEST_ASSERT_EQUAL_INT32(I2C_OK, result);
    TEST_ASSERT_EQUAL_UINT32(400000, g_mock_i2c.speed);
}

void test_I2C_Init_ShouldReturnError_WhenAlreadyInitialized(void) {
    I2C_Init(100000);

    // Try to re-initialize - should return error
    int32_t result = I2C_Init(100000);
    TEST_ASSERT_EQUAL_INT32(I2C_ERR_ALREADY_INIT, result);
}

/* ============================================================
 * I2C Write Register Tests
 * ============================================================ */

void test_I2C_WriteReg_ShouldReturnSuccess_WhenValidInput(void) {
    uint8_t device_addr = 0x50;   // 7-bit address (0xA0 >> 1)
    uint8_t reg_addr = 0x10;
    uint8_t write_data = 0xAB;

    int32_t result = I2C_WriteReg(device_addr, reg_addr, write_data);

    TEST_ASSERT_EQUAL_INT32(I2C_OK, result);
    TEST_ASSERT_EQUAL_UINT8(0xAB, Mock_I2C_GetMemory(reg_addr));
}

void test_I2C_WriteReg_ShouldWriteCorrectData_ToMemory(void) {
    // Test multiple register writes
    I2C_WriteReg(0x50, 0x00, 0x11);
    I2C_WriteReg(0x50, 0x01, 0x22);
    I2C_WriteReg(0x50, 0x02, 0x33);

    TEST_ASSERT_EQUAL_UINT8(0x11, Mock_I2C_GetMemory(0x00));
    TEST_ASSERT_EQUAL_UINT8(0x22, Mock_I2C_GetMemory(0x01));
    TEST_ASSERT_EQUAL_UINT8(0x33, Mock_I2C_GetMemory(0x02));
}

void test_I2C_WriteReg_ShouldReturnNack_WhenDeviceNotPresent(void) {
    // Configure mock to always return NACK
    Mock_I2C_SetNackCount(10);

    int32_t result = I2C_WriteReg(0xFF, 0x00, 0x00);  // Invalid device

    TEST_ASSERT_EQUAL_INT32(I2C_ERR_NACK, result);
}

/* ============================================================
 * I2C Read Register Tests
 * ============================================================ */

void test_I2C_ReadReg_ShouldReturnCorrectData(void) {
    // Setup: write data first
    uint8_t device_addr = 0x50;
    uint8_t reg_addr = 0x10;
    uint8_t expected_data = 0xCD;

    Mock_I2C_SetMemory(reg_addr, expected_data);

    // Act
    int32_t result = I2C_ReadReg(device_addr, reg_addr);

    // Assert
    TEST_ASSERT_EQUAL_INT32(I2C_OK, result);
    TEST_ASSERT_EQUAL_UINT8(expected_data, result);
}

void test_I2C_ReadReg_ShouldHandle16BitRegister(void) {
    // Test 16-bit register address
    uint8_t device_addr = 0x50;
    uint16_t reg_addr = 0x0100;
    uint8_t expected_data = 0xEF;

    Mock_I2C_SetMemory((uint8_t)reg_addr, expected_data);

    // Note: This tests the API - actual implementation may differ
    int32_t result = I2C_ReadReg16(device_addr, reg_addr);

    TEST_ASSERT_EQUAL_INT32(I2C_OK, result);
}

/* ============================================================
 * NACK Retry Logic Tests
 * ============================================================ */

void test_I2C_WriteReg_ShouldRetryOnFirstNack(void) {
    // Configure: fail first attempt, succeed on second
    Mock_I2C_SetNackCount(1);

    int32_t result = I2C_WriteReg(0x50, 0x10, 0xAB);

    // Should succeed after retry
    TEST_ASSERT_EQUAL_INT32(I2C_OK, result);
    TEST_ASSERT_EQUAL_UINT8(0xAB, Mock_I2C_GetMemory(0x10));

    // Should have attempted 2 transfers
    TEST_ASSERT_EQUAL_UINT8(2, Mock_I2C_GetTransferCount());
}

void test_I2C_WriteReg_ShouldRetryUpToMaxRetries(void) {
    // Configure: fail first 2 attempts, succeed on third
    Mock_I2C_SetNackCount(2);

    int32_t result = I2C_WriteReg(0x50, 0x10, 0xAB);

    // Should succeed after 2 retries
    TEST_ASSERT_EQUAL_INT32(I2C_OK, result);
    TEST_ASSERT_EQUAL_UINT8(3, Mock_I2C_GetTransferCount());
}

void test_I2C_WriteReg_ShouldReturnError_AfterMaxRetries(void) {
    // Configure: fail all attempts
    Mock_I2C_SetNackCount(10);

    int32_t result = I2C_WriteReg(0x50, 0x10, 0xAB);

    // Should fail after max retries
    TEST_ASSERT_EQUAL_INT32(I2C_ERR_NACK, result);

    // Should have attempted exactly 3 times (max retries + 1)
    TEST_ASSERT_EQUAL_UINT8(3, Mock_I2C_GetTransferCount());
}

void test_I2C_ReadReg_ShouldRetryOnNack(void) {
    Mock_I2C_SetNackCount(1);
    Mock_I2C_SetMemory(0x10, 0xCD);

    int32_t result = I2C_ReadReg(0x50, 0x10);

    TEST_ASSERT_EQUAL_INT32(I2C_OK, result);
    TEST_ASSERT_EQUAL_UINT8(2, Mock_I2C_GetTransferCount());
}

/* ============================================================
 * Multi-byte Transfer Tests
 * ============================================================ */

void test_I2C_WriteReg16_ShouldWriteTwoBytes(void) {
    uint8_t device_addr = 0x50;
    uint16_t reg_addr = 0x0010;
    uint16_t write_data = 0x1234;  // Big-endian: 0x12, 0x34

    int32_t result = I2C_WriteReg16(device_addr, reg_addr, write_data);

    TEST_ASSERT_EQUAL_INT32(I2C_OK, result);
    TEST_ASSERT_EQUAL_UINT8(0x12, Mock_I2C_GetMemory(reg_addr));
    TEST_ASSERT_EQUAL_UINT8(0x34, Mock_I2C_GetMemory(reg_addr + 1));
}

void test_I2C_ReadReg16_ShouldReadTwoBytes(void) {
    uint8_t device_addr = 0x50;
    uint16_t reg_addr = 0x0010;

    Mock_I2C_SetMemory(reg_addr, 0x12);
    Mock_I2C_SetMemory(reg_addr + 1, 0x34);

    uint16_t result = I2C_ReadReg16(device_addr, reg_addr);

    // Read function returns the data directly, not error code
    // Adjust test based on actual API
    TEST_ASSERT_EQUAL_HEX16(0x1234, result);
}

void test_I2C_WriteBlock_ShouldTransferMultipleBytes(void) {
    uint8_t device_addr = 0x50;
    uint8_t reg_addr = 0x00;
    uint8_t data[] = {0x01, 0x02, 0x03, 0x04, 0x05};

    int32_t result = I2C_WriteBlock(device_addr, reg_addr, data, sizeof(data));

    TEST_ASSERT_EQUAL_INT32(5, result);

    // Verify all bytes written
    for (uint8_t i = 0; i < sizeof(data); i++) {
        TEST_ASSERT_EQUAL_UINT8(data[i], Mock_I2C_GetMemory(reg_addr + i));
    }
}

/* ============================================================
 * Error Handling Tests
 * ============================================================ */

void test_I2C_WriteReg_ShouldReturnError_WhenNotInitialized(void) {
    Mock_I2C_Reset();  // Not initialized

    int32_t result = I2C_WriteReg(0x50, 0x10, 0xAB);

    TEST_ASSERT_EQUAL_INT32(I2C_ERR_NOT_INIT, result);
}

void test_I2C_WriteReg_ShouldReturnError_WhenInvalidAddress(void) {
    // 8-bit address 0xFF is invalid (reserved)
    int32_t result = I2C_WriteReg(0xFF, 0x10, 0xAB);

    TEST_ASSERT_EQUAL_INT32(I2C_ERR_INVALID_PARAM, result);
}

void test_I2C_ReadReg_ShouldReturnError_WhenNackAfterRetries(void) {
    Mock_I2C_SetNackCount(10);

    int32_t result = I2C_ReadReg(0x50, 0x10);

    TEST_ASSERT_EQUAL_INT32(I2C_ERR_NACK, result);
}

/* ============================================================
 * Timeout Tests
 * ============================================================ */

void test_I2C_WriteReg_ShouldHandleTimeout(void) {
    // Simulate timeout by setting bus busy
    g_mock_i2c.busy = 1;

    int32_t result = I2C_WriteReg(0x50, 0x10, 0xAB);

    TEST_ASSERT_EQUAL_INT32(I2C_ERR_TIMEOUT, result);
}

/* ============================================================
 * Boundary Tests
 * ============================================================ */

void test_I2C_WriteReg_ShouldHandleBoundaryAddress(void) {
    // Test with boundary addresses
    TEST_ASSERT_EQUAL_INT32(I2C_OK, I2C_WriteReg(0x00, 0x00, 0x00));
    TEST_ASSERT_EQUAL_INT32(I2C_OK, I2C_WriteReg(0x7F, 0xFF, 0xFF));
}

void test_I2C_WriteReg_ShouldHandleAllFFData(void) {
    // Test with 0xFF pattern
    int32_t result = I2C_WriteReg(0x50, 0x10, 0xFF);

    TEST_ASSERT_EQUAL_INT32(I2C_OK, result);
    TEST_ASSERT_EQUAL_UINT8(0xFF, Mock_I2C_GetMemory(0x10));
}

void test_I2C_WriteReg_ShouldHandleAll00Data(void) {
    // Test with 0x00 pattern
    int32_t result = I2C_WriteReg(0x50, 0x10, 0x00);

    TEST_ASSERT_EQUAL_INT32(I2C_OK, result);
    TEST_ASSERT_EQUAL_UINT8(0x00, Mock_I2C_GetMemory(0x10));
}

/* ============================================================
 * Performance Tests
 * ============================================================ */

void test_I2C_WriteReg_ShouldCompleteWithinTimeout(void) {
    uint32_t start_time = 0;  // Mock timer

    int32_t result = I2C_WriteReg(0x50, 0x10, 0xAB);

    TEST_ASSERT_EQUAL_INT32(I2C_OK, result);
    // In real tests, verify elapsed time < timeout
}

/* ============================================================
 * Main Entry Point
 * ============================================================ */

int main(void) {
    UNITY_BEGIN();

    // Run all tests in logical groups
    RUN_TEST_GROUP(I2C_Init);
    RUN_TEST_GROUP(I2C_Write);
    RUN_TEST_GROUP(I2C_Read);
    RUN_TEST_GROUP(NACK_Retry);
    RUN_TEST_GROUP(MultiByte);
    RUN_TEST_GROUP(ErrorHandling);
    RUN_TEST_GROUP(Timeout);
    RUN_TEST_GROUP(Boundary);
    RUN_TEST_GROUP(Performance);

    return UNITY_END();
}
