#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"

void init_uart();

int main()
{
    stdio_init_all();
    init_uart_pins();
    printf("hi");
    return 0;
}