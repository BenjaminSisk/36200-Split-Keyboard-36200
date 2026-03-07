#pragma once
#include "pico/stdlib.h"
#include <string>
#include <cstdio>
#include "../KeypadButtons/keypadButtons.h"
#include "keyMap.h"
#include "Joysticks/PicoJoystick.h"


class TerminalEmulator {
public:
    /**
     * @brief Parameterized constructor linking the emulator to physical peripheral instances.
     * @param keypad Ref to the keypad driver to inject button states.
     * @param joystick Ref to the joystick driver to inject analog states.
     */
    TerminalEmulator(KeypadButtons& keypad, PicoJoystick& joystick);

    /**
     * @brief Non-blocking UART reader and parser. Call continuously in the main loop.
     */
    void update();

private:
    KeypadButtons& keypadRef;
    PicoJoystick& joystickRef;
    std::string rxBuffer;

    /**
     * @brief Parses the completed command string and executes the simulation.
     */
    void parseCommand();


    //flag: if true, prints out a waiting for user input message. 
    // else (most cases), do not print. 
    bool isUserNeedsPrompt = true;
};