/**
 * @file inputHandler.cpp 
 */
#include "inputHandler.h"
#include "hardware/sync.h"
#include <cstdio>
#include <uart_comm.h>

// Constructor no longer takes arguments
InputHandler::InputHandler() : matrix(), joystick() {
    equipmentState.fill(0);

    equipmentState[hardwareMap::LEFT_JOY_X_ID] = 127;
    equipmentState[hardwareMap::LEFT_JOY_Y_ID] = 127;
    equipmentState[hardwareMap::RIGHT_JOY_X_ID] = 127;
    equipmentState[hardwareMap::RIGHT_JOY_Y_ID] = 127;
    
    // Rely solely on compile-time flag
    if (hardwareMap::IS_LEFT_HALF) {
        joyIdX  = hardwareMap::LEFT_JOY_X_ID;
        joyIdY  = hardwareMap::LEFT_JOY_Y_ID;
        joyIdSw = hardwareMap::LEFT_JOY_SW_ID;
    } else {
        joyIdX  = hardwareMap::RIGHT_JOY_X_ID;
        joyIdY  = hardwareMap::RIGHT_JOY_Y_ID;
        joyIdSw = hardwareMap::RIGHT_JOY_SW_ID;
    }

    matrix.setOnChange([this](uint8_t index, bool isPressed) { 
        this->handleKeyChange(index, isPressed); 
    });

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
        
        // FIX 1: Explicitly poll the local switch state and push updates if it changes
        bool sw_pressed = joystick.getPressed();
        uint8_t sw_val = sw_pressed ? 1 : 0;
        if (equipmentState[joyIdSw] != sw_val) {
            enqueueEvent(joyIdSw, sw_val);
        }
    }



    if (!hardwareMap::IS_LEFT_HALF) {
        uint8_t byte1, byte2;
        if (uart_read_pair(&byte1, &byte2)) {
            enqueueEvent(byte1, byte2);
        }
    }

    flushQueueToFifo();
}

void InputHandler::handleKeyChange(uint8_t buttonIndex, bool isPressed) {
    uint8_t row = buttonIndex / hardwareMap::COLS;
    uint8_t col = buttonIndex % hardwareMap::COLS;
    
    // FIX: Shift column index for the right half to read correctly from global matrixToId
    if (!hardwareMap::IS_LEFT_HALF) {
        col += 6; 
    }
    
    uint8_t equipmentId = hardwareMap::matrixToId[row][col];
    
    if (equipmentId != hardwareMap::NO_CONN) {
        enqueueEvent(equipmentId, isPressed ? 1 : 0);
    }
}


void InputHandler::enqueueEvent(uint8_t equipmentId, uint8_t actionValue) {
    // 1. Update the steady state source of truth safely
    equipmentState[equipmentId] = actionValue;

    // 2. Dispatch to other half if this is the peripheral keyboard
    if (hardwareMap::IS_LEFT_HALF) {
        keyboard_uart_send(equipmentId, actionValue); 
    } 

    // 3. Enqueue for Core 1 (LED processing) safely
    uint32_t payload = (static_cast<uint32_t>(equipmentId) << 8) | actionValue;
    uint32_t ints = save_and_disable_interrupts();
    internalQueue.push(payload);
    restore_interrupts(ints); 
}

uint8_t InputHandler::getEquipmentState(uint8_t equipmentId) const {
    return equipmentState[equipmentId];
}

std::vector<uint8_t> InputHandler::getActiveEquipmentIds() const {
    std::vector<uint8_t> activeIds;
    
    // Scan the absolute state array (safely avoids out of bounds)
    for (size_t i = 0; i < 256; i++) {
        // Skip IDs that belong to the analog joysticks or NO_CONN 
        // to prevent HID keyboard from trying to type analog data
        if (i == hardwareMap::NO_CONN || 
            i == hardwareMap::LEFT_JOY_X_ID || i == hardwareMap::LEFT_JOY_Y_ID ||
            i == hardwareMap::RIGHT_JOY_X_ID || i == hardwareMap::RIGHT_JOY_Y_ID ||
            i == hardwareMap::LEFT_JOY_SW_ID || i == hardwareMap::RIGHT_JOY_SW_ID) {
            continue; 
        }
        
        if (equipmentState[i] > 0) {
            activeIds.push_back(static_cast<uint8_t>(i));
        }
    }
    
    return activeIds;
}

void InputHandler::flushQueueToFifo() {
    while (true) {
        uint32_t payload;
        bool hasData = false;

        uint32_t ints = save_and_disable_interrupts();
        if (!internalQueue.empty()) {
            payload = internalQueue.front();
            internalQueue.pop();
            hasData = true;
        }
        restore_interrupts(ints); 
        
        if (!hasData) break; 

        if (multicore_fifo_wready()) {
            multicore_fifo_push_blocking(payload);
        }
    }
}

void InputHandler::debugPrint() const {
    uint32_t current_ms = to_ms_since_boot(get_absolute_time());
    if (current_ms - last_print_ms >= print_interval_ms) {
        last_print_ms = current_ms;

        // Grab the absolute array state for both joysticks directly from memory
        uint8_t lx = equipmentState[hardwareMap::LEFT_JOY_X_ID];
        uint8_t ly = equipmentState[hardwareMap::LEFT_JOY_Y_ID];
        uint8_t lsw = equipmentState[hardwareMap::LEFT_JOY_SW_ID];
        
        uint8_t rx = equipmentState[hardwareMap::RIGHT_JOY_X_ID];
        uint8_t ry = equipmentState[hardwareMap::RIGHT_JOY_Y_ID];
        uint8_t rsw = equipmentState[hardwareMap::RIGHT_JOY_SW_ID];

        // Fetch keyboard IDs
        std::vector<uint8_t> activeIds = getActiveEquipmentIds();
        std::string activeIdStr = "";
        for (uint8_t id : activeIds) {
            activeIdStr += std::to_string(id) + " ";
        }
        if (activeIdStr.empty()) activeIdStr = "NONE";

        // Output a clean, single-line state
        printf("\r[SYS] L_JOY(X:%3d Y:%3d SW:%d) | R_JOY(X:%3d Y:%3d SW:%d) | IDs: [%s]          ", 
               lx, ly, lsw, rx, ry, rsw, activeIdStr.c_str());
        fflush(stdout);
    }
}

void InputHandler::debugPrintOld() const {
    uint32_t current_ms = to_ms_since_boot(get_absolute_time());
    if (current_ms - last_print_ms >= print_interval_ms) {
        last_print_ms = current_ms;

        char joyBuffer[80]; 
        joystick.toString(joyBuffer, sizeof(joyBuffer));
        std::string keyString = matrix.toString(); 

        printf("\r[SYS] %s | KEYS: [%s]", joyBuffer, keyString.c_str());
        fflush(stdout);
    }
}
//