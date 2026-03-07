// include/config/systemHardware.h
#pragma once
#include "KeypadButtons/keypadButtons.h"
#include "Joysticks/PicoJoystick.h"
#include "config/hardwareMap.h"
/**
 * @brief Container for all physical hardware components on the split keyboard.
 */
struct SystemHardware {
    KeypadButtons keypad;
    PicoJoystick leftJoy;
    PicoJoystick rightJoy; 

    // Constructor utilizes the new hardwareMap::Pins namespace
    SystemHardware() : 
        // Convert the constexpr std::arrays into std::vectors for the keypad
        keypad(std::vector<uint8_t>(hardwareMap::Pins::KEYBOARD_ROW_PINS.begin(), hardwareMap::Pins::KEYBOARD_ROW_PINS.end()),
               std::vector<uint8_t>(hardwareMap::Pins::KEYBOARD_COL_PINS.begin(), hardwareMap::Pins::KEYBOARD_COL_PINS.end())),
        leftJoy(hardwareMap::Pins::LEFT_JOY_X, 0, 
                hardwareMap::Pins::LEFT_JOY_Y, 1, 
                hardwareMap::Pins::LEFT_JOY_SW, {true, true, false}, -10, 0.2f),
        
        rightJoy(hardwareMap::Pins::RIGHT_JOY_X, 2, // Dummy ADC channels
                 hardwareMap::Pins::RIGHT_JOY_Y, 3, 
                 hardwareMap::Pins::RIGHT_JOY_SW, {false, false, false}, -10, 0.2f) {}

    void initAll() {
        keypad.init();
        // Only initialize hardware if pins are actually defined
        if (hardwareMap::Pins::LEFT_JOY_X != hardwareMap::NO_PIN) leftJoy.init();
        if (hardwareMap::Pins::RIGHT_JOY_X != hardwareMap::NO_PIN) rightJoy.init();
    }
};