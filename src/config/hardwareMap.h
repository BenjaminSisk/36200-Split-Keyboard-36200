#pragma once
#include <array>
#include <cstdint>

namespace hardwareMap {
    constexpr uint8_t ROWS = 4;
    constexpr uint8_t COLS = 12;

    /**
     * @brief 2D array mapping physical matrix coordinates [row][col] to Equipment IDs.
     * Empty physical spaces in the matrix (e.g., gaps near thumb clusters) map to 255.
     */
    constexpr std::array<std::array<uint8_t, COLS>, ROWS> matrixToId = {{
        // Left Half (0-5)                 // Right Half (6-11)
        {0,  1,  2,  3,  4,  5,            6,  7,  8,  9,  10, 11},  // Row 0
        {12, 13, 14, 15, 16, 17,           18, 19, 20, 21, 22, 23},  // Row 1
        {24, 25, 26, 27, 28, 29,           30, 31, 32, 33, 34, 35},  // Row 2
        {255, 36, 37, 38, 255, 255,        255, 39, 40, 41, 255, 255} // Row 3 (Thumbs)
    }};

    // Standard Equipment IDs for Joysticks
    constexpr uint8_t LEFT_JOY_X = 128;
    constexpr uint8_t LEFT_JOY_Y = 129;
    constexpr uint8_t RIGHT_JOY_X = 130;
    constexpr uint8_t RIGHT_JOY_Y = 131;
}