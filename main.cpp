#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "subsystems/joysticks/PicoJoystick.h"

int main() {
    stdio_init_all();
    adc_init();

    printf("\033[2J"); 
    printf("System Initialized. Starting IRQs...\n");

    // Initialize using constructor delegation defaults
    PicoJoystick joystick;
    joystick.init();
    
    // Fire up the hardware timer IRQ
    joystick.startTimer();

    // The main loop is now entirely decoupled from sensor polling latency
    while (true) {
        // Output visualization using the filtered data
        joystick.debugPrintSingleLine();
        
        // This sleep no longer blocks sensor reading!
        sleep_ms(100); 
    }
    
    return 0;
}