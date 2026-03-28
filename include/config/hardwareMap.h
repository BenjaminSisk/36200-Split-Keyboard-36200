#pragma once
#include <array>
#include <cstdint>



namespace hardwareMap {
    constexpr bool IS_LEFT_HALF = true; // Set to false for right half firmware builds. This can be used for conditional compilation if needed, but ideally the same codebase should work for both halves with the same pin definitions. 

    //The constexpr specifier in C++ indicates that a variable or function can be evaluated at compile time. This shifts computations from runtime to compile time, which allows for performance optimizations, earlier bug detection via compile-time checks
    // both halfs should use the same GPIO pins. 
    // this is for one half. 
    constexpr uint8_t ROWS = 4;
    constexpr uint8_t COLS = 6;
    constexpr uint8_t TOTAL_BUTTONS = ROWS * COLS; // 24 buttons per half, 48 total

    // Define a standard "unconnected" pin state
    constexpr uint8_t NO_PIN = 255; 
    constexpr uint8_t NO_CONN = 255; // For matrixToId mapping - no counection

    namespace Pins {
        // Left Joystick
        // constexpr uint8_t LEFT_JOY_SW = NO_PIN; //42;
        // constexpr uint8_t LEFT_JOY_Y  = NO_PIN; //41;
        // constexpr uint8_t LEFT_JOY_X  = NO_PIN; //40;

        // // Right Joystick (Currently unconnected/virtual)
        // constexpr uint8_t RIGHT_JOY_SW = NO_PIN;
        // constexpr uint8_t RIGHT_JOY_Y  = NO_PIN;
        // constexpr uint8_t RIGHT_JOY_X  = NO_PIN;

        //joysticks
        constexpr uint8_t JOYSTICK_SW = 42;
        constexpr uint8_t JOYSTICK_Y  = 41;
        constexpr uint8_t JOYSTICK_X  = 40;

        // --- Keypad Matrix ---
        // row pins
        constexpr std::array<uint8_t, ROWS> KEYBOARD_ROW_PINS = { 2, 3, 4, 5 };
        
        // column pins
        constexpr std::array<uint8_t, COLS> KEYBOARD_COL_PINS = { 
            6, 7, 8, 9, 10, 11                             // Cols 0-5
        };
    }
       
    /**
     * @brief 2D array mapping physical matrix coordinates [row][col] to Equipment IDs.
     * Empty physical spaces in the matrix (e.g., gaps near thumb clusters) map to 255.
     */
    constexpr std::array<std::array<uint8_t, 12>, 4> matrixToId = {{
        // Left Half (0-5)                 // Right Half (6-11)
        {0,  1,  2,  3,  4,  5,            6,  7,  8,  9,  10, 11},  // Row 0
        {12, 13, 14, 15, 16, 17,           18, 19, 20, 21, 22, 23},  // Row 1
        {24, 25, 26, 27, 28, 29,           30, 31, 32, 33, 34, 35},  // Row 2
        {255, 36, 37, 38, 255, 255,        255, 39, 40, 41, 255, 255} // Row 3 (Thumbs)
    }};

    constexpr std::array<std::array<uint8_t, 6>, 4> BUTTON_POSITION_TO_ID_LEFT = {{
        {0,  1,  2,  3,  4,  5},   // Row 0
        {12, 13, 14, 15, 16, 17},   // Row 1
        {24, 25, 26, 27, 28, 29},   // Row 2
        {255, 36, 37, 38, 255, 255} // Row 3 (Thumbs)
    }};

    constexpr std::array<std::array<uint8_t, 6>, 4> BUTTON_POSITION_TO_ID_RIGHT = {{
        {6,  7,  8,  9,  10, 11},   // Row 0
        {18, 19, 20, 21, 22, 23},   // Row 1
        {30, 31, 32, 33, 34, 35},   // Row 2
        {255, 39, 40, 41, 255, 255} // Row 3 (Thumbs)
    }};




    // Standard Equipment IDs for Joysticks
    constexpr uint8_t LEFT_JOY_X_ID = 128;
    constexpr uint8_t LEFT_JOY_Y_ID = 129;
    constexpr uint8_t RIGHT_JOY_X_ID = 130;
    constexpr uint8_t RIGHT_JOY_Y_ID = 131;
}