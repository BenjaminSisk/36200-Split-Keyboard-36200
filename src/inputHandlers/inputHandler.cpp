/**
 * @file inputHandler.cpp
 * @brief Implementation of InputHandler configured for dynamic split-keyboard handedness.
 */
#include "inputHandler.h"
#include "hardware/sync.h"
#include <cstdio>

InputHandler::InputHandler(bool isLeftHalf) : matrix(), joystick() {
    
    if (isLeftHalf) {
        joyIdX = hardwareMap::LEFT_JOY_X_ID;
        joyIdY = hardwareMap::LEFT_JOY_Y_ID;
        matrixMapping = &hardwareMap::BUTTON_POSITION_TO_ID;
    } else {
        joyIdX = hardwareMap::RIGHT_JOY_X_ID;
        joyIdY = hardwareMap::RIGHT_JOY_Y_ID;
        matrixMapping = &hardwareMap::BUTTON_POSITION_TO_ID;
    }

    // WHAT: Bind the unified matrix callback.
    // WHY: We map the boolean 'isPressed' state directly through to our new handler.
    matrix.setOnChange([this](uint8_t index, bool isPressed) { 
        this->handleKeyChange(index, isPressed); 
    });

    // WHAT: Bind the unified joystick callback.
    // WHY: We use a ternary operator to instantly resolve the correct ID based on the Axis enum.
    joystick.setOnChange([this](PicoJoystick::Axis axis, uint8_t val) { 
        uint8_t targetId = (axis == PicoJoystick::Axis::X) ? this->joyIdX : this->joyIdY;
        this->enqueueEvent(targetId, val); 
    });

    last_print_ms = 0;

}

void InputHandler::init() {
    matrix.init();
    joystick.init(false);
}

void InputHandler::update() {
    uint32_t now = time_us_32();

    if (now - last_matrix_us >= 5000) {
        last_matrix_us = now;
        matrix.update();
    }

    if (now - last_joystick_us >= 10000) {
        last_joystick_us = now;
        joystick.update();
    }

    flushQueueToFifo();
}

// WHAT: The new unified key handler.
// WHY: We convert the 'isPressed' boolean into a 1 or 0 action value in a single line, 
// eliminating the need for duplicate functions.
void InputHandler::handleKeyChange(uint8_t buttonIndex, bool isPressed) {
    uint8_t row = buttonIndex / hardwareMap::COLS;
    uint8_t col = buttonIndex % hardwareMap::COLS;
    
    uint8_t equipmentId = (*matrixMapping)[row][col];
    
    if (equipmentId != hardwareMap::NO_CONN) {
        // Ternary operator: if isPressed is true, pass 1. Else, pass 0.
        enqueueEvent(equipmentId, isPressed ? 1 : 0);
    }
}

void InputHandler::debugPrint() const {
    //nonblocking print

    uint32_t current_ms = to_ms_since_boot(get_absolute_time());
    if (current_ms - last_print_ms >= print_interval_ms) {
        last_print_ms = current_ms;

        char joyBuffer[80]; 
        joystick.toString(joyBuffer, sizeof(joyBuffer));

        // Get the string and pass its underlying C-string directly to printf
        std::string keyString = matrix.toString(); 

        // \r moves the cursor to the start of the line.
        printf("\rSYS: [%s] | KEYS: [%s]        ", joyBuffer, keyString.c_str());
        
        // Force the console to output immediately
        fflush(stdout);

    } else {
        return; // Skip this print to maintain the interval
    }

}


void InputHandler::enqueueEvent(uint8_t equipmentId, uint8_t actionValue) {
    uint32_t payload = (static_cast<uint32_t>(equipmentId) << 8) | actionValue;
    // printf("[DEBUG] Enqueuing Event - Equipment ID: %d, Action Value: %d\n", equipmentId, actionValue);

    // Pause interrupts to safely push to the queue
    uint32_t ints = save_and_disable_interrupts();
    internalQueue.push(payload);
    restore_interrupts(ints); // Resume interrupts
}

uint16_t InputHandler::getJoystickX() const { return joystick.getX(); }
uint16_t InputHandler::getJoystickY() const { return joystick.getY(); }
bool InputHandler::getJoystickPressed() const { return joystick.getPressed(); }

void InputHandler::flushQueueToFifo() {
    while (true) {
        uint32_t payload;
        bool hasData = false;

        // Pause interrupts to safely pop from the queue
        uint32_t ints = save_and_disable_interrupts();
        if (!internalQueue.empty()) {
            payload = internalQueue.front();
            internalQueue.pop();
            hasData = true;
        }
        restore_interrupts(ints); // Resume interrupts

        if (!hasData) break; // Queue is empty, exit loop

        if (multicore_fifo_wready()) {
            multicore_fifo_push_blocking(payload);
        }
    }
}



std::vector<uint8_t> InputHandler::getActiveEquipmentIds() const {
    std::vector<uint8_t> activeIds;
    
    // Scan the entire matrix state
    for (uint8_t i = 0; i < hardwareMap::TOTAL_BUTTONS; i++) {
        if (matrix.isKeyPressed(i)) {
            // Translate the raw index back into Row/Col
            uint8_t row = i / hardwareMap::COLS;
            uint8_t col = i % hardwareMap::COLS;
            
            // Look up the mapped ID
            uint8_t equipmentId = (*matrixMapping)[row][col];
            
            if (equipmentId != hardwareMap::NO_CONN) {
                activeIds.push_back(equipmentId);
            }
        }
    }
    return activeIds;
}