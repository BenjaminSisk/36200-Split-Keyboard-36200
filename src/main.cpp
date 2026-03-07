#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

#include "config/systemHardware.h"
#include "InputHandlers/inputHandler.h"
#include "InputEmulators/terminalEmulator.h"

#include "InputHandlers/inputHandler.h"
#include "InputEmulators/terminalEmulator.h"
#include "KeypadButtons/keypadButtons.h"
#include "Joysticks/PicoJoystick.h"

#include "quertyMap.h"

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


// --- Core 1 Task ---
void core1Task() {
    printf("[DEBUG] Core 1 Task Booted and waiting...\n"); // <-- ADD THIS BEFORE THE LOOP

    while (true) {
        // Block core 1 until data arrives from the handler
        uint32_t rawData = multicore_fifo_pop_blocking();

        printf("[DEBUG] Core 1 popped raw data: %lu\n", rawData); //debug

        uint8_t equipmentId = (rawData >> 8) & 0xFF;
        uint8_t actionValue = rawData & 0xFF;

        if (equipmentId < 128) {
            // It's a keypress! Route it through the QWERTY flash configuration.
            char mappedChar = qwertyMap::getChar(equipmentId);
            printf("ID: %d | Char: %c | State: %s\n", 
                   equipmentId, mappedChar, actionValue ? "PRESSED" : "RELEASED");
        } else {
            // It's an analog or peripheral input
            printf("Analog ID: %d | Value: %d\n", equipmentId, actionValue);
        }
    }
}

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include <stdio.h>


// ... (core1Task remains exactly the same) ...

int main() {
    stdio_init_all();
    sleep_ms(2000); // Allow USB serial to initialize
    
    printf("\n=== System Booting ===\n");

    // 1. Initialize Hardware FIRST
    printf("Initializing Hardware...\n");
    SystemHardware hw;
    hw.initAll(); 

    // 2. Inject hardware dependencies into handlers
    InputHandler systemInputs(hw.keypad, hw.leftJoy, hw.rightJoy);
    TerminalEmulator emulator(hw.keypad, hw.leftJoy);

    // 3. Launch Core 1 LAST to prevent printf race conditions
    printf("Launching Core 1 Task...\n");
    multicore_launch_core1(core1Task);

    //Always initialize your system memory, hardware components, and configuration variables on Core 0 first, and make launching Core 1 the absolute final step before entering your while(true) loop. This prevents race conditions and ensures Core 1 boots into a fully stable environment.
    sleep_ms(1.5); // Give core 1.5s a moment to boot and print its message
    printf("Boot complete...\n");

    while (true) {
        // High-frequency polling tasks
        // printf("[DEBUG] Main Loop Cycle Started\n"); // debug

        // printf("[DEBUG] Polling System Inputs...\n"); // debug
        systemInputs.update();
        
        // Listen for user terminal commands non-blockingly
        // THIS IS REQUIRED for your emulator to actually work!
        // printf("[DEBUG] Polling Terminal Emulator...\n"); // debug
        emulator.update();
    }
        
        // Now you can easily pass the hardware into your visualizer
        // debug_visualizer(true, hw.leftJoy, false, false, false); 
    

    return 0;
}