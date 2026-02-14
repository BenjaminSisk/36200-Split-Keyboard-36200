#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"

#define UART_ID uart1
#define UART_TX_PIN 4
#define UART_RX_PIN 5
#define BAUD_RATE 115200

void init_uart_pins()
{
    
    gpio_set_function(UART_TX_PIN, UART_FUNCSEL_NUM(UART_ID,UART_TX_PIN));
    gpio_set_function(UART_RX_PIN, UART_FUNCSEL_NUM(UART_ID,UART_RX_PIN));
    uart_init(UART_ID, BAUD_RATE);
    uart_set_fifo_enabled(UART_ID, false);
    irq_set_exclusive_handler(UART_IRQ_NUM(UART_ID), uart_rx_isr);
    irq_set_enabled(UART_IRQ_NUM(UART_ID),true);
    uart_set_irqs_enabled(UART_ID, true, false);
}   

void uart_rx_isr()
{
    gpio_acknowledge_irq(5, UART)
}