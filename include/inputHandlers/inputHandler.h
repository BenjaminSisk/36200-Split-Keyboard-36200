#pragma once
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "KeypadButtons/keypadButtons.h"
#include "Joysticks/PicoJoystick.h"
#include "config/hardwareMap.h"
#include <cstdint>

class InputHandler {
public:
    /**
     * @brief Default constructor.
     * Delegates to the parameterized constructor using default peripheral allocations.
     */
    InputHandler();

    /**
     * @brief Parameterized constructor associating the handler with specific physical hardware interfaces.
     * @param keypad Reference to the primary keypad scanning subsystem.
     * @param leftJoy Reference to the left joystick subsystem.
     * @param rightJoy Reference to the right joystick subsystem.
     */
    InputHandler(KeypadButtons& keypad, PicoJoystick& leftJoy, PicoJoystick& rightJoy);
    /**
     * @brief Polls all active input peripherals and processes data non-blockingly.
     * Must be called continuously in the main execution loop.
     */
    void update();

private:
    KeypadButtons& matrixRef;
    PicoJoystick& leftJoyRef;
    PicoJoystick& rightJoyRef;

    // State tracking to prevent sending duplicate analog data
    uint8_t lastLeftX = 0;
    uint8_t lastLeftY = 0;
    uint8_t lastRightX = 0;
    uint8_t lastRightY = 0;

    /**
     * @brief Callback invoked by the matrix scanner when a button is pressed.
     * @param buttonIndex The 1D hardware index calculated by the keypad driver.
     */
    void handleKeyPress(uint8_t buttonIndex);

    /**
     * @brief Callback invoked by the matrix scanner when a button is released.
     * @param buttonIndex The 1D hardware index calculated by the keypad driver.
     */
    void handleKeyRelease(uint8_t buttonIndex);

    /**
     * @brief Encodes the ID and Action into a single 32-bit word and pushes it to Core 1.
     * @param equipmentId The predefined ID for the specific hardware piece.
     * @param actionValue The digital state (1/0) or 8-bit analog payload.
     */
    void dispatchToQueue(uint8_t equipmentId, uint8_t actionValue);
};