#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

#include "inputHandler.h"
#include "inputEmulators/terminalEmulator.h"
#include "keypadButtons/keypadButtons.h"
#include "joysticks/PicoJoystick.h"


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
    while (true) {
        // Block core 1 until data arrives from the handler
        uint32_t rawData = multicore_fifo_pop_blocking();

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

    multicore_launch_core1(core1Task);

    // Instantiate Hardware
    KeypadButtons physicalKeypad;
    PicoJoystick leftJoystick; 
    
    // Instantiate Handler and Emulator
    InputHandler systemInputs(physicalKeypad, leftJoystick);
    TerminalEmulator emulator(physicalKeypad, leftJoystick);

    physicalKeypad.init();

    printf("Boot Complete. Type 'k 12 1' to simulate button 12 press.\n");

    while (true) {
        // High-frequency polling tasks
        systemInputs.update();
        
        // Listen for user terminal commands non-blockingly
        emulator.update();
    }

    return 0;
}