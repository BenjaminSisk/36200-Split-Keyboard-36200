#include "terminalEmulator.h"

TerminalEmulator::TerminalEmulator(KeypadButtons& keypad, PicoJoystick& joystick)
    : keypadRef(keypad), joystickRef(joystick) {}

void TerminalEmulator::update() {
    // Non-blocking read from stdio (UART/USB Serial)
    int c = getchar_timeout_us(0);

    if (c != PICO_ERROR_TIMEOUT) {
        if (c == '\n' || c == '\r') {
            if (!rxBuffer.empty()) {
                parseCommand();
                rxBuffer.clear(); // Reset buffer after execution
            }
        } else {
            // Append character to buffer (safeguard against buffer overflow)
            if (rxBuffer.length() < 32) {
                rxBuffer += static_cast<char>(c);
            }
        }
    }
}

void TerminalEmulator::parseCommand() {
    uint8_t arg1, arg2;

    if (rxBuffer[0] == 'k') {
        // Parse keypad command: "k <index> <1/0>"
        if (sscanf(rxBuffer.c_str(), "k %hhu %hhu", &arg1, &arg2) == 2) {
            printf("[Emulator] Keypad %d -> %s\n", arg1, arg2 ? "PRESSED" : "RELEASED");
            keypadRef.simulateState(arg1, arg2 > 0);
        } else {
            printf("[Emulator] Invalid Keypad Command. Use: k [index] [1/0]\n");
        }
    } 
    else if (rxBuffer[0] == 'j') {
        // Parse joystick command: "j <x> <y>"
        if (sscanf(rxBuffer.c_str(), "j %hhu %hhu", &arg1, &arg2) == 2) {
            printf("[Emulator] Joystick X:%d Y:%d\n", arg1, arg2);
            joystickRef.simulatePosition(arg1, arg2);
        } else {
            printf("[Emulator] Invalid Joystick Command. Use: j [x] [y]\n");
        }
    }
}