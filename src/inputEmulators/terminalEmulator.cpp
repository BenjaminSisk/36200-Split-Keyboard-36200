#include "terminalEmulator.h"

TerminalEmulator::TerminalEmulator(KeypadButtons& keypad, PicoJoystick& joystick)
    : keypadRef(keypad), joystickRef(joystick) {}

void TerminalEmulator::update() {
    // Non-blocking read from stdio (UART/USB Serial)
    int c = getchar_timeout_us(0); //

    if (c != PICO_ERROR_TIMEOUT) { //
        // 1. Handle Enter/Return keys
        if (c == '\n' || c == '\r') { //
            if (!rxBuffer.empty()) { //
                printf("\n"); // Echo the newline to move cursor down
                parseCommand(); //
                rxBuffer.clear(); // Reset buffer after execution
            }
        } 
        // 2. Handle Backspace (ASCII 8 or 127 depending on terminal)
        else if (c == '\b' || c == 127) {
            if (!rxBuffer.empty()) { //
                rxBuffer.pop_back();
                
                // Visually erase the character on the user's terminal:
                // Move cursor back (\b), print a space to overwrite, move back again (\b)
                printf("\b \b"); 
                fflush(stdout); //visible immediately
            }
        }
        // 3. Handle normal printable characters
        else if (c >= 32 && c <= 126) {
            // Append character to buffer (safeguard against buffer overflow)
            if (rxBuffer.length() < 32) { //
                rxBuffer += static_cast<char>(c); //
                
                // CRITICAL FIX: Echo the character back to the terminal
                putchar(c); 
                fflush(stdout); //Force the typed character to send immediately
            }
        }
    }
}

void TerminalEmulator::parseCommand() {
    int arg1, arg2; //user args for parsing commands that require parameters (e.g., button index, joystick x/y values)

    // Provide the help menu
    if (rxBuffer == "help" || rxBuffer == "h") {
        printf("\n=== Terminal Emulator Help ===\n");
        printf("k [id] [1/0] : Simulate Keypad (e.g., 'k 12 1' for press)\n");
        printf("j [x] [y]    : Simulate Joystick (e.g., 'j 128 128')\n");
        printf("help         : Print this menu\n");
        printf("==============================\n");
        return;
    }

    if (rxBuffer[0] == 'k') { //
        // Parse keypad command: "k <index> <1/0>"
        if (sscanf(rxBuffer.c_str(), "k %d %d", &arg1, &arg2) == 2) {
            printf("[Emulator] Keypad %d -> %s\n", arg1, arg2 ? "PRESSED" : "RELEASED");
            
            // Cast the safely parsed integers back down to uint8_t for the hardware
            keypadRef.simulateState(static_cast<uint8_t>(arg1), arg2 > 0);

        } else {
            printf("[Emulator] Invalid Keypad Command. Use: k [index] [1/0]\n"); //
        }
    } 
    else if (rxBuffer[0] == 'j') { //
        // Parse joystick command: "j <x> <y>"
        if (sscanf(rxBuffer.c_str(), "j %d %d", &arg1, &arg2) == 2) {
            printf("[Emulator] Joystick X:%d Y:%d\n", arg1, arg2);
            
            // Cast back down to uint8_t
            joystickRef.simulatePosition(static_cast<uint8_t>(arg1), static_cast<uint8_t>(arg2));
    
        } else {
            printf("[Emulator] Invalid Joystick Command. Use: j [x] [y]\n"); //
        }
    } 
    else {
        // Fallback for unrecognized input
        printf("[Emulator] Unknown command: '%s'. Type 'help' for options.\n", rxBuffer.c_str());
    }
}