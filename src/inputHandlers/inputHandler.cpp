/**
 * @file inputHandler.cpp
 * @brief Implementation of InputHandler configured for dynamic split-keyboard handedness.
 */
#include "inputHandler.h"
#include <cstdio>

InputHandler::InputHandler(bool isLeftHalf) : matrix(), joystick() {
    
    // 1. Assign local identity based on the passed boolean
    if (isLeftHalf) {
        joyIdX = hardwareMap::LEFT_JOY_X_ID;
        joyIdY = hardwareMap::LEFT_JOY_Y_ID;
        matrixMapping = &hardwareMap::BUTTON_POSITION_TO_ID_LEFT;
    } else {
        joyIdX = hardwareMap::RIGHT_JOY_X_ID;
        joyIdY = hardwareMap::RIGHT_JOY_Y_ID;
        matrixMapping = &hardwareMap::BUTTON_POSITION_TO_ID_RIGHT;
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

}

void InputHandler::init() {
    matrix.init();
    joystick.init();

    startHardwareTimers();
}

void InputHandler::startHardwareTimers() {
    add_repeating_timer_us(-5000, keypadTimerCallback, this, &keypadTimer);  
    add_repeating_timer_us(-10000, joystickTimerCallback, this, &joystickTimer); 
}

bool InputHandler::keypadTimerCallback(struct repeating_timer *t) {
    InputHandler* instance = static_cast<InputHandler*>(t->user_data);
    if (instance) instance->matrix.update();
    return true; 
}

bool InputHandler::joystickTimerCallback(struct repeating_timer *t) {
    InputHandler* instance = static_cast<InputHandler*>(t->user_data);
    if (instance) instance->joystick.update();
    return true; 
}

void InputHandler::update() {
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
    // 1. Allocate fixed-size buffers on the stack (Extremely fast, zero heap fragmentation)
    char joyBuffer[80]; 
    char keyBuffer[32]; // Needs at least 25 bytes (24 buttons + 1 null terminator)

    // 2. Command the subclasses to populate their strings
    joystick.toString(joyBuffer, sizeof(joyBuffer));
    matrix.toString(keyBuffer, sizeof(keyBuffer));

    // 3. Print the master line. 
    // \r moves the cursor to the start of the line.
    // The extra spaces at the end ensure that if the string gets shorter, old characters are erased.
    printf("\rSYS: [%s] | KEYS: [%s]       ", joyBuffer, keyBuffer);
    
    // 4. Force the console to output immediately, since we aren't using \n
    fflush(stdout);
}

void InputHandler::enqueueEvent(uint8_t equipmentId, uint8_t actionValue) {
    // printf("[DEBUG] Enqueuing Event - Equipment ID: %d, Action Value: %d\n", equipmentId, actionValue);
    uint32_t payload = (static_cast<uint32_t>(equipmentId) << 8) | actionValue;
    internalQueue.push(payload);
}

void InputHandler::flushQueueToFifo() {
    while (!internalQueue.empty()) {
        if (multicore_fifo_wready()) {
            uint32_t payload = internalQueue.front();
            multicore_fifo_push_blocking(payload);
            internalQueue.pop(); 
        } else {
            break; 
        }
    }
}