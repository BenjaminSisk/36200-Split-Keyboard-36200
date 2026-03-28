/**
 * @file inputHandler.h
 * @brief Manages input peripherals, aggregates state, and dispatches events.
 * * Owns the joystick and keypad components via direct composition. Uses an 
 * internal queue to buffer and filter events before dispatching them non-blockingly
 * to the multicore FIFO.
 */
#pragma once

#include <cstdint>
#include <queue>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "keypadButtons.h"
#include "PicoJoystick.h"
#include "hardwareMap.h"

class InputHandler {
public:
    /**
     * @brief Constructs the handler and binds hardware callbacks.
     * @param isLeftHalf True if this firmware is running on the left physical half.
     */
    InputHandler(bool isLeftHalf);    
    /**
     * @brief Starts the hardware timers for ISR-based polling.
     */
    void startHardwareTimers();

    void init(); // universal init

    /**
     * @brief Drains the internal queue. Called in main execution loop.
     */
    void update();


    /**
     * @brief Prints a continuously updating, single-line debug state of all managed peripherals.
     * Overwrites the previous line in the console using a carriage return.
     */
    void debugPrint() const;

private:
    // Physical hardware components owned entirely by InputHandler
    KeypadButtons matrix;
    PicoJoystick joystick; // Identical physical component for both halves

    std::queue<uint32_t> internalQueue;

    // --- State variables for Handedness Identity ---
    uint8_t joyIdX;
    uint8_t joyIdY;
    
    // Pointer to the correct 2D mapping array in hardwareMap
    const std::array<std::array<uint8_t, hardwareMap::COLS>, hardwareMap::ROWS>* matrixMapping;    
    
    // Hardware Timers for IRQ polling
    struct repeating_timer keypadTimer;
    struct repeating_timer joystickTimer;

    // Static IRQ callbacks
    static bool keypadTimerCallback(struct repeating_timer *t);
    static bool joystickTimerCallback(struct repeating_timer *t);

    void handleKeyChange(uint8_t buttonIndex, bool isPressed);

    /**
     * @brief Formats an event and pushes it to the internal C++ queue.
     * @param equipmentId Predefined ID for the specific hardware piece.
     * @param actionValue Digital state (1/0) or 8-bit analog payload.
     */
    void enqueueEvent(uint8_t equipmentId, uint8_t actionValue);

    /**
     * @brief Attempts to drain the internal queue into the hardware multicore FIFO.
     * Will stop safely if the hardware FIFO becomes full to prevent blocking.
     */
    void flushQueueToFifo();
};