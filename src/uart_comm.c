#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"

#define UART_ID0 uart0 //manager
#define UART_TX0_PIN 0 
#define UART_RX0_PIN 1
#define BAUD_RATE 115200

#define UART_ID1 uart1 //subordinate
#define UART_TX1_PIN 36
#define UART_RX1_PIN 37


/*
void uart_rx_isr()
{
    gpio_acknowledge_irq(5, UART_IRQ_NUM(UART_ID));
    
}
*/

void init_uart_pins()
{
    
    gpio_set_function(UART_TX0_PIN, UART_FUNCSEL_NUM(UART_ID0,UART_TX0_PIN));
    gpio_set_function(UART_RX0_PIN, UART_FUNCSEL_NUM(UART_ID0,UART_RX0_PIN));
    uart_init(UART_ID0, BAUD_RATE);
    uart_set_fifo_enabled(UART_ID0, false);
    
    gpio_set_function(UART_TX1_PIN, UART_FUNCSEL_NUM(UART_ID1,UART_TX1_PIN));
    gpio_set_function(UART_RX1_PIN, UART_FUNCSEL_NUM(UART_ID1,UART_RX1_PIN));
    uart_init(UART_ID1, BAUD_RATE);
    uart_set_fifo_enabled(UART_ID1, false);
    
    //irq_set_exclusive_handler(UART_IRQ_NUM(UART_ID), uart_rx_isr);
    //irq_set_enabled(UART_IRQ_NUM(UART_ID),true);
    //uart_set_irqs_enabled(UART_ID, true, false);
}   

