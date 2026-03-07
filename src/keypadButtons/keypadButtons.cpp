#include "KeypadButtons.h"

KeypadButtons::KeypadButtons(const std::vector<uint>& rowPins, const std::vector<uint>& colPins) 
    : rows(rowPins), cols(colPins), lastUpdateUs(0) {
    
    int totalButtons = rows.size() * cols.size();
    validatedState.resize(totalButtons, false);
    debounceCounters.resize(totalButtons, 0);
}

void KeypadButtons::init() {
    // Initialize Rows (Horizontal) as Inputs with Pull-Downs
    for (uint pin : rows) {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
        gpio_pull_down(pin); // Keeps the pin at logic 0 when no button is pressed
    }

    // Initialize Columns (Vertical) as Outputs, driven low initially
    for (uint pin : cols) {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_OUT);
        gpio_put(pin, 0);
    }
}

void KeypadButtons::update() {
    uint32_t currentUs = time_us_32();
    
    // Non-blocking wait: only scan every SCAN_INTERVAL_US (e.g., 5ms)
    if (currentUs - lastUpdateUs < SCAN_INTERVAL_US) {
        return; 
    }
    lastUpdateUs = currentUs;

    uint8_t totalCols = cols.size();

    // Scan through each column
    for (size_t c = 0; c < cols.size(); c++) {
        // Drive the current column high
        gpio_put(cols[c], 1);
        
        // Brief blocking delay to let the voltage stabilize (capacitance settling)
        // 2 microseconds is plenty and won't stall the main loop
        sleep_us(2); 

        // Read all rows for this column
        for (size_t r = 0; r < rows.size(); r++) {
            bool isPressed = gpio_get(rows[r]);
            
            // Calculate the 1D index: (Row * Total Columns) + Col
            uint8_t index = (r * totalCols) + c;

            // Debounce Logic State Machine
            if (isPressed != validatedState[index]) {
                debounceCounters[index]++;
                
                if (debounceCounters[index] >= DEBOUNCE_THRESHOLD) {
                    // State has stabilized, accept the new state
                    validatedState[index] = isPressed;
                    debounceCounters[index] = 0;

                    // Fire the appropriate callback
                    if (isPressed && onKeyPressCb) {
                        onKeyPressCb(index);
                    } else if (!isPressed && onKeyReleaseCb) {
                        onKeyReleaseCb(index);
                    }
                }
            } else {
                // If the reading matches the validated state, reset the bounce counter
                debounceCounters[index] = 0;
            }
        }

        // Drive the column back low before moving to the next one
        gpio_put(cols[c], 0);
    }
}

void KeypadButtons::setOnKeyPress(std::function<void(uint8_t)> callback) {
    onKeyPressCb = callback;
}

void KeypadButtons::setOnKeyRelease(std::function<void(uint8_t)> callback) {
    onKeyReleaseCb = callback;
}