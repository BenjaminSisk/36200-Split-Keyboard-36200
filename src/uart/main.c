#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <uart_comm.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"



int main()
{
    
    init_uart_pins();
    init_uart_isr();
    for (;;) {
        char buf[2];
        //char buff;
        //printf("AHHHHHHHHHHHHHHHHH");
        //uart_read_blocking(uart0, (uint8_t*)buf, sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0'; // Ensure null-termination
        //uart_puts(uart1, "You said: ");
        //uart_putc(uart0, buf[0]);
        //uart_puts(uart0, "\n");
        
       
    }
    
}