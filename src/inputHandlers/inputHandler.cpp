#include "inputHandler.h"


//default values for the default constructor to avoid null reference issues. 
//These will never be used in practice since the default constructor is just a delegate to the parameterized one, 
//but they need to exist to satisfy the compiler.
static KeypadButtons defaultKeypad;
static PicoJoystick defaultLeftJoy; 
static PicoJoystick defaultRightJoy;   

InputHandler::InputHandler() : InputHandler(defaultKeypad, defaultLeftJoy, defaultRightJoy) {}

InputHandler::InputHandler(KeypadButtons& keypad, PicoJoystick& leftJoy, PicoJoystick& rightJoy)
    : matrixRef(keypad), leftJoyRef(leftJoy), rightJoyRef(rightJoy) {
    
    // Bind hardware matrix callbacks to this class
    matrixRef.setOnKeyPress([this](uint8_t index) { this->handleKeyPress(index); });
    matrixRef.setOnKeyRelease([this](uint8_t index) { this->handleKeyRelease(index); });
}

void InputHandler::update() {
    // printf("[DEBUG] InputHandler Update Cycle Started\n"); // debug

    // 1. Non-blocking poll of the digital keyboard matrix
    // printf("[DEBUG] Polling Keypad Matrix...\n"); // debug
    matrixRef.update();

    // 2. Non-blocking poll of analog joysticks
    // printf("[DEBUG] Polling Joysticks...\n"); // debug
    leftJoyRef.update();
    rightJoyRef.update();

    // printf("[DEBUG] Fetching Joystick States...\n"); // debug
    uint8_t currentLeftX = leftJoyRef.getX();
    uint8_t currentLeftY = leftJoyRef.getY();
    uint8_t currentRightX = rightJoyRef.getX();
    uint8_t currentRightY = rightJoyRef.getY();
    // printf("[DEBUG] Joystick States - LeftX: %d, LeftY: %d, RightX: %d, RightY: %d\n", 
    //         currentLeftX, currentLeftY, currentRightX, currentRightY); // debug

    // Only dispatch analog data if it has changed (reduces FIFO noise)
    if (currentLeftX != lastLeftX) {
        dispatchToQueue(hardwareMap::LEFT_JOY_X_ID, currentLeftX);
        lastLeftX = currentLeftX;
    }

    if (currentLeftY != lastLeftY) {
        dispatchToQueue(hardwareMap::LEFT_JOY_Y_ID, currentLeftY);
        lastLeftY = currentLeftY;
    }

    if (currentRightX != lastRightX) {
        dispatchToQueue(hardwareMap::RIGHT_JOY_X_ID, currentRightX);
        lastRightX = currentRightX;
    }

    if (currentRightY != lastRightY) {
        dispatchToQueue(hardwareMap::RIGHT_JOY_Y_ID, currentRightY);
        lastRightY = currentRightY;
    }
}

void InputHandler::handleKeyPress(uint8_t buttonIndex) {
    // Convert 1D hardware index to row/col based on the matrix math
    uint8_t row = buttonIndex / hardwareMap::COLS;
    uint8_t col = buttonIndex % hardwareMap::COLS;

    uint8_t equipmentId = hardwareMap::matrixToId[row][col];
    
    printf("[DEBUG] Callback received. Row: %d, Col: %d, EquipID: %d\n", row, col, equipmentId); //debug log to verify correct mapping

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
        printf("[DEBUG] FIFO Ready. Pushing payload: %lu\n", payload); // <-- ADD THIS
        multicore_fifo_push_blocking(payload); 
    } else {
        printf("[DEBUG ERROR] FIFO is FULL or locked!\n"); // <-- ADD THIS
    }

}