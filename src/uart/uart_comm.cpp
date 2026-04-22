/**
 * @file uart-comm.cpp
 * @brief Implementation of basic uart to be used for bluetooth communication betwen keyboard halves
 */

#include "uart_comm.h"
#include <stdio.h>

// Ring buffer for UART1 (Subordinate data)
static struct {
    uint8_t buffer[RING_BUFFER_SIZE];
    volatile uint16_t head;
    volatile uint16_t tail;
} rb1 = {0};

volatile uint32_t parity_error_count = 0;
volatile uint8_t last_valid_byte = 0;

void init_uart_pins() {
    
    uart_set_format(UART_ID0, 8, 1, UART_PARITY_EVEN);
   
    uart_set_fifo_enabled(UART_ID0, false);

    uart_init(UART_ID1, BAUD_RATE);
    gpio_set_function(UART_TX1_PIN, UART_FUNCSEL_NUM(UART_ID1, UART_TX1_PIN));
    gpio_set_function(UART_RX1_PIN, UART_FUNCSEL_NUM(UART_ID1, UART_RX1_PIN));
        
    uart_set_format(UART_ID1, 8, 1, UART_PARITY_EVEN);
    uart_set_fifo_enabled(UART_ID1, false);
}

/**
 * Checks for hardware errors before returning the byte.
 * Returns true if data is valid, false if parity/framing error occurred.
 */
bool try_read_uart(uart_inst_t *uart, uint8_t *dest) {
    uart_hw_t *hw = uart_get_hw(uart);

    // Check Receive Status Register (RSR) for Parity Error (PE) or Framing Error (FE)
    if (hw->rsr & (UART_UARTRSR_PE_BITS | UART_UARTRSR_FE_BITS)) {
        parity_error_count++;
        hw->rsr = 0;           // Clear the error bits in the hardware
        (void)uart_getc(uart); // Read and discard the corrupt byte from the FIFO
        return false; 
    }

    if (uart_is_readable(uart)) {
        *dest = uart_getc(uart);
        return true;
    }
    
    return false;
}

// ISR for UART 0 (Manager)
void on_uart0_rx() {
    uint8_t data;
    if (try_read_uart(UART_ID0, &data)) {
        last_valid_byte = data;
    }
}

// ISR for UART 1 (Subordinate)
void on_uart1_rx() {
    uint8_t data;
    if (try_read_uart(UART_ID1, &data)) {
        last_valid_byte = data;
        
        // Push to ring buffer
        uint16_t next = (rb1.head + 1) % RING_BUFFER_SIZE;
        if (next != rb1.tail) {
            rb1.buffer[rb1.head] = data;
            rb1.head = next;
        }
    }
}

void init_uart_isr() {
    // --- UART 1 Interrupt setup ---
    irq_set_exclusive_handler(UART1_IRQ, on_uart1_rx);
    irq_set_enabled(UART1_IRQ, true);
    // Enable the RX interrupt
    uart_set_irqs_enabled(UART_ID1, true, false);
}

void keyboard_uart_send(uint8_t byte1, uint8_t byte2) {
    uart_putc(UART_ID1, byte1);
    uart_putc(UART_ID1, byte2);
}

bool uart_read_pair(uint8_t *byte1, uint8_t *byte2) {
    static uint8_t first_byte = 0;
    static bool waiting_for_second = false;

    while (rb1.head != rb1.tail) {
        uint8_t data = rb1.buffer[rb1.tail];
        rb1.tail = (rb1.tail + 1) % RING_BUFFER_SIZE;

        if (!waiting_for_second) {
            first_byte = data;
            waiting_for_second = true;
        } else {
            *byte1 = first_byte;
            *byte2 = data;
            waiting_for_second = false;
            return true;
        }
    }
    return false;
}