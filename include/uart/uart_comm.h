#ifndef UART_HANDLER_H
#define UART_HANDLER_H
#define RING_BUFFER_SIZE 256

#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "hardware/structs/uart.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define UART_ID0 uart0
#define UART_TX0_PIN 0
#define UART_RX0_PIN 1

#define UART_ID1 uart1
#define UART_TX1_PIN 40
#define UART_RX1_PIN 41

#define BAUD_RATE 115200

extern volatile uint32_t parity_error_count;
extern volatile uint8_t last_valid_byte;

/// @brief initializes uart gpio
void init_uart_pins(void);

/// @brief initializes uart isr
void init_uart_isr(void);

/// @brief checks parity
/// @param uart
/// @param dest
/// @return true if no parity/framing error
bool try_read_uart(uart_inst_t *uart, uint8_t *dest);

/// @brief Returns data from the uart buffer
/// @param address to store byte 1
/// @param address to store byte 2
/// @return true succesful
bool uart_read_pair(uint8_t *byte1, uint8_t *byte2);

/// @brief Sends two bytes over UART
/// @param byte1 First byte to send
/// @param byte2 Second byte to send
void keyboard_uart_send(uint8_t byte1, uint8_t byte2);

#endif