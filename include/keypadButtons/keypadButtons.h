#pragma once
#include "pico/stdlib.h"
#include "hardwareMap.h"
#include <vector>
#include <functional>
#include <cstdint>

class KeypadButtons {
public:

    //initializer
    //uses hardwareMap for all pin definitions, so no need to pass in pin vectors. 
    KeypadButtons();

    // Setup GPIO pins
    void init();

    // Call this continuously in your main loop
    void update();

    // Register callbacks passing the raw 1D button index
    void setOnChange(std::function<void(uint8_t, bool)> callback);

    /**
     * @brief Manually injects a button state for emulation, bypassing hardware and debounce.
     * @param buttonIndex The 1D hardware index of the button (0 to N-1).
     * @param isPressed True for pressed, false for released.
     */
    void simulateState(uint8_t buttonIndex, bool isPressed);

    /**
     * @brief Writes a visual representation of the matrix state into a provided buffer.
     * @param buffer Pointer to a character array.
     * @param maxLength The maximum number of bytes that can be written safely.
     */
    void toString(char* buffer, size_t maxLength) const;


private:
    //row and col stored in hardwareMap. 

    uint32_t lastUpdateUs;
    const uint32_t SCAN_INTERVAL_US = 5000;   // Scan the whole matrix every 5ms
    const uint8_t DEBOUNCE_THRESHOLD = 4;     // Requires 4 consecutive identical reads (20ms)
    
    std::array<bool, hardwareMap::TOTAL_BUTTONS> buttonState; // Final debounced state of each button
    std::array<uint8_t, hardwareMap::TOTAL_BUTTONS> debounceCounters;

    std::function<void(uint8_t, bool)> onChangeCb;

};