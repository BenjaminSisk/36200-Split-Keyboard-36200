#include "InputHandlers/inputHandler.h"

static KeypadButtons defaultKeypad;
static PicoJoystick defaultLeftJoy; // Assuming PicoJoystick exists in your joysticks folder

InputHandler::InputHandler() : InputHandler(defaultKeypad, defaultLeftJoy) {}

InputHandler::InputHandler(KeypadButtons& keypad, PicoJoystick& leftJoy)
    : matrixRef(keypad), leftJoyRef(leftJoy) {
    
    // Bind hardware matrix callbacks to this class
    matrixRef.setOnKeyPress([this](uint8_t index) { this->handleKeyPress(index); });
    matrixRef.setOnKeyRelease([this](uint8_t index) { this->handleKeyRelease(index); });
}

void InputHandler::update() {
    // 1. Non-blocking poll of the digital keyboard matrix
    matrixRef.update();

    // 2. Non-blocking poll of analog joysticks
    leftJoyRef.update();
    
    uint8_t currentLeftX = leftJoyRef.getX();
    uint8_t currentLeftY = leftJoyRef.getY();

    // Only dispatch analog data if it has changed (reduces FIFO noise)
    if (currentLeftX != lastLeftX) {
        dispatchToQueue(hardwareMap::LEFT_JOY_X, currentLeftX);
        lastLeftX = currentLeftX;
    }

    if (currentLeftY != lastLeftY) {
        dispatchToQueue(hardwareMap::LEFT_JOY_Y, currentLeftY);
        lastLeftY = currentLeftY;
    }
}

void InputHandler::handleKeyPress(uint8_t buttonIndex) {
    // Convert 1D hardware index to row/col based on the matrix math
    uint8_t row = buttonIndex / hardwareMap::COLS;
    uint8_t col = buttonIndex % hardwareMap::COLS;

    uint8_t equipmentId = hardwareMap::matrixToId[row][col];
    
    // 255 means empty physical matrix slot; ignore it
    if (equipmentId != 255) {
        dispatchToQueue(equipmentId, 1);
    }
}

void InputHandler::handleKeyRelease(uint8_t buttonIndex) {
    uint8_t row = buttonIndex / hardwareMap::COLS;
    uint8_t col = buttonIndex % hardwareMap::COLS;

    uint8_t equipmentId = hardwareMap::matrixToId[row][col];
    
    if (equipmentId != 255) {
        dispatchToQueue(equipmentId, 0);
    }
}

void InputHandler::dispatchToQueue(uint8_t equipmentId, uint8_t actionValue) {
    uint32_t payload = (static_cast<uint32_t>(equipmentId) << 8) | actionValue;

    // Check if Core 1 is ready before pushing to avoid stalling main loop
    if (multicore_fifo_wready()) {
        multicore_fifo_push_blocking(payload); 
    }
}