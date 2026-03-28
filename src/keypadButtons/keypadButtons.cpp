/**
 * @file keypadButtons.cpp
 * @brief Implementation of the KeypadButtons driver.
 */

#include "keypadButtons.h"
#include "hardwareMap.h" // Needed ONLY to read NO_PIN
#include <cstdio> // For snprintf in toString()

#define KEYBOARD_ROW_PINS hardwareMap::Pins::KEYBOARD_ROW_PINS
#define KEYBOARD_COL_PINS hardwareMap::Pins::KEYBOARD_COL_PINS

KeypadButtons::KeypadButtons() {
    // Initialize button states and debounce counters
    buttonState.fill(false);
    debounceCounters.fill(0);
}


void KeypadButtons::init() {
    // Initialize Rows (Horizontal) as Inputs with Pull-Downs
    for (uint pin : KEYBOARD_ROW_PINS) {
        if (pin == hardwareMap::NO_PIN) continue; // Skip virtual rows
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
        gpio_pull_down(pin); // Keeps the pin at logic 0 when no button is pressed
    }

    // Initialize Columns (Vertical) as Outputs, driven low initially
    for (uint pin : KEYBOARD_COL_PINS) {
        if (pin == hardwareMap::NO_PIN) continue; // Skip virtual cols
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_OUT);
        gpio_put(pin, 0);
    }

}

void KeypadButtons::update() {
    // Removed the time_us_32() check. We rely on the caller (IRQ) for timing.
    uint8_t totalCols = KEYBOARD_COL_PINS.size();

    for (size_t c = 0; c < KEYBOARD_COL_PINS.size(); c++) {
        if (KEYBOARD_COL_PINS[c] != hardwareMap::NO_PIN) {
            gpio_put(KEYBOARD_COL_PINS[c], 1);
            busy_wait_us(2); //You cannot wait for an interrupt while you are actively inside an interrupt. T
        }

        for (size_t r = 0; r < KEYBOARD_ROW_PINS.size(); r++) {
            if (KEYBOARD_COL_PINS[c] == hardwareMap::NO_PIN || 
                KEYBOARD_ROW_PINS[r] == hardwareMap::NO_PIN) {
                continue; 
            }

            bool isPressed = gpio_get(KEYBOARD_ROW_PINS[r]);
            uint8_t index = (r * totalCols) + c;

            if (isPressed != buttonState[index]) {
                debounceCounters[index]++;
                
                if (debounceCounters[index] >= DEBOUNCE_THRESHOLD) {
                    buttonState[index] = isPressed;
                    debounceCounters[index] = 0;

                    printf("[DEBUG] Matrix Index %d changed. Pressed: %d\n", index, isPressed);

                    // WHAT: Fire the single unified callback, passing the state as an argument.
                    // WHY: Consolidates the event pipeline.
                    if (onChangeCb) {
                        onChangeCb(index, isPressed);
                    }
                }
            } else {
                debounceCounters[index] = 0;
            }
        }
        gpio_put(KEYBOARD_COL_PINS[c], 0);
    }
}

void KeypadButtons::setOnChange(std::function<void(uint8_t, bool)> callback) {
    onChangeCb = callback;
}


void KeypadButtons::simulateState(uint8_t buttonIndex, bool isPressed) {
    if (buttonIndex >= buttonState.size()) return;

    // Only fire if the simulated state differs from the current validated state
    if (buttonState[buttonIndex] != isPressed) {
        buttonState[buttonIndex] = isPressed;
        
        if (isPressed && onChangeCb) {
            onChangeCb(buttonIndex, isPressed);
        } else if (!isPressed && onChangeCb) {
            onChangeCb(buttonIndex, isPressed);
        }
    }
}

void KeypadButtons::toString(char* buffer, size_t maxLength) const {
    // Safety check: Ensure the buffer is large enough for 24 buttons + null terminator
    if (maxLength < hardwareMap::TOTAL_BUTTONS + 1) return;
    
    for (size_t i = 0; i < hardwareMap::TOTAL_BUTTONS; i++) {
        // '#' represents a pressed button, '-' represents released
        buffer[i] = buttonState[i] ? '#' : '-';
    }
    
    // Always manually null-terminate C-strings
    buffer[hardwareMap::TOTAL_BUTTONS] = '\0';
}