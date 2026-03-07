#pragma once
#include "pico/stdlib.h"
#include <vector>
#include <functional>
#include <cstdint>

class KeypadButtons {
public:
    // Pass hardware pins via constructor
    KeypadButtons(const std::vector<uint>& rowPins, const std::vector<uint>& colPins);

    // Default constructor with testing configs. Override with parameterized constructor for production.
    KeypadButtons();

    // Setup GPIO pins
    void init();

    // Call this continuously in your main loop
    void update();

    // Register callbacks passing the raw 1D button index
    void setOnKeyPress(std::function<void(uint8_t)> callback);
    void setOnKeyRelease(std::function<void(uint8_t)> callback);

    /**
     * @brief Manually injects a button state for emulation, bypassing hardware and debounce.
     * @param buttonIndex The 1D hardware index of the button (0 to N-1).
     * @param isPressed True for pressed, false for released.
     */
    void simulateState(uint8_t buttonIndex, bool isPressed);

private:
    std::vector<uint> rows;
    std::vector<uint> cols;
    
    uint32_t lastUpdateUs;
    const uint32_t SCAN_INTERVAL_US = 5000;   // Scan the whole matrix every 5ms
    const uint8_t DEBOUNCE_THRESHOLD = 4;     // Requires 4 consecutive identical reads (20ms)
    
    std::vector<bool> validatedState;
    std::vector<uint8_t> debounceCounters;

    std::function<void(uint8_t)> onKeyPressCb;
    std::function<void(uint8_t)> onKeyReleaseCb;
};