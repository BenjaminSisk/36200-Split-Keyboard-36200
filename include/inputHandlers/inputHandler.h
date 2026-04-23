/**
 * @file inputHandler.h
 * @brief Manages input peripherals, aggregates state, and dispatches events.
 */
#pragma once

#include <cstdint>
#include <queue>
#include <vector> 
#include <array>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "keypadButtons.h"
#include "PicoJoystick.h"
#include "hardwareMap.h"

class InputHandler {
public:
    // Removed boolean argument; it will rely solely on hardwareMap::IS_LEFT_HALF
    InputHandler();    
    void init();
    void update();
    void debugPrint() const;
    void debugPrintOld() const; 

    // --- Unified State Getters ---
    uint8_t getEquipmentState(uint8_t equipmentId) const;
    std::vector<uint8_t> getActiveEquipmentIds() const;

private:
    KeypadButtons matrix;
    PicoJoystick joystick; 

    std::queue<uint32_t> internalQueue;

    // --- Local Hardware IDs ---
    uint8_t joyIdX;
    uint8_t joyIdY;
    uint8_t joyIdSw; // Added to track local switch ID
    
    // THE STEADY STATE
    std::array<uint8_t, 256> equipmentState;

    uint32_t last_matrix_us   = 0;
    uint32_t last_joystick_us = 0;
    mutable uint32_t last_print_ms = 0; 
    const uint32_t print_interval_ms = 50; 

    void handleKeyChange(uint8_t buttonIndex, bool isPressed);
    void enqueueEvent(uint8_t equipmentId, uint8_t actionValue);
    void flushQueueToFifo();
};