#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

// Include future subsystems here
#include "subsystems/joysticks/PicoJoystick.h"

/**
 * @brief Centralized visualizer callback for all active subsystems.
 * * @note If multiple subsystems print to the terminal simultaneously, 
 * single-line carriage returns (\r) will overwrite each other. As you 
 * enable more modules, you may need to transition to ANSI multi-line 
 * cursor positioning.
 */
void debug_visualizer(bool isJoystick, const PicoJoystick& joystick, 
                      bool isLeds /*, const Leds& leds*/, 
                      bool isComm /*, const Comm& comm*/, 
                      bool isUsb /*, const Usb& usb*/) {
    
    if (isJoystick) {
        joystick.debugPrint();
    }
    
    if (isLeds) {
        // leds.debugPrint();
    }
    
    if (isComm) {
        // comm.debugPrint();
    }
    
    if (isUsb) {
        // usb.debugPrint();
    }
}

int main() {
    stdio_init_all();
    adc_init();

    printf("\033[2J"); 
    printf("System Initialized. Starting IRQs...\n");

    // Instantiate subsystems
    PicoJoystick joystick;
    
    // Rapid testing initialization
    joystick.testing_QuickInit();

    // The main loop is now entirely decoupled from sensor polling latency
    while (true) {
        
        // Call the unified visualizer
        // Set flags to true to enable specific subsystem terminal output
        debug_visualizer(true, joystick, false, false, false);
        
        sleep_ms(100); 


        // This sleep no longer blocks sensor reading!
        sleep_ms(100); 
    }
    
    return 0;
}