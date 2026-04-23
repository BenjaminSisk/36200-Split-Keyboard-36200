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
    InputHandler(bool isLeftHalf);    
    void init();
    void update();
    void debugPrint() const;
    void debugPrintOld() const; // Legacy method for single-line joystick print, can be removed later

    // --- Unified State Getters ---
    // Returns 0-255 (analog) or 0/1 (digital) for ANY hardware ID
    uint8_t getEquipmentState(uint8_t equipmentId) const;
    std::vector<uint8_t> getActiveEquipmentIds() const;

private:
    KeypadButtons matrix;
    PicoJoystick joystick; 

    std::queue<uint32_t> internalQueue;

    uint8_t joyIdX;
    uint8_t joyIdY;
    const std::array<std::array<uint8_t, hardwareMap::COLS>, hardwareMap::ROWS>* matrixMapping;    
    
    // THE STEADY STATE: Single source of truth for both local and remote halves
    // Index = Equipment ID. Value = 0/1 (Digital) or 0-255 (Analog)
    std::array<uint8_t, 256> equipmentState;

    uint32_t last_matrix_us   = 0;
    uint32_t last_joystick_us = 0;
    mutable uint32_t last_print_ms = 0; 
    const uint32_t print_interval_ms = 50; 

    void handleKeyChange(uint8_t buttonIndex, bool isPressed);

    /**
     * @brief Updates the steady state array and dispatches to queues/UART.
     */
    void enqueueEvent(uint8_t equipmentId, uint8_t actionValue);
    void flushQueueToFifo();
};