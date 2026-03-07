#include <unity.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "uart_comm.h"

// Define timeout to prevent hanging if hardware fails
#define UART_TIMEOUT_MS 100

void setUp(void)
{
    // Reset or initialize hardware state before each test
}

void tearDown(void)
{
    // Clean up or disable interrupts if necessary
}

void test_uart_transmission(void)
{
    const char test_char = 'A';
    char received_char = 0;

    // Send data using your SDK wrapper or direct SDK calls
    uart_putc(UART_ID, test_char);

    // Wait for data with a timeout to avoid blocking the test runner
    uint32_t start = to_ms_since_boot(get_absolute_time());
    while (!uart_is_readable(UART_ID))
    {
        if (to_ms_since_boot(get_absolute_time()) - start > UART_TIMEOUT_MS)
        {
            TEST_FAIL_MESSAGE("UART Receive Timeout: Check loopback jumper");
            return;
        }
    }

    received_char = uart_getc(UART_ID);

    // Unity assertion to verify the data
    TEST_ASSERT_EQUAL_INT8(test_char, received_char);
}

void test_buffer_logic(void)
{
    char buf[5] = "test";
    // Example of testing your local buffer logic
    TEST_ASSERT_EQUAL_STRING("test", buf);
    TEST_ASSERT_NOT_NULL(buf);
}

int main()
{
    // RP2350 SDK initialization
    stdio_init_all();

    // Essential for PlatformIO to capture Unity output over Serial
    sleep_ms(2000);

    // Your specific UART setup from uart_comm.h
    init_uart_pins();
    init_uart_isr();

    UNITY_BEGIN();

    RUN_TEST(test_uart_transmission);
    RUN_TEST(test_buffer_logic);

    // Unity will send a final summary over the serial port
    return UNITY_END();
}