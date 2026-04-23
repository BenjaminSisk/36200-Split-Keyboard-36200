#pragma once
#include <array>
#include <cstdint>

namespace qwertyMap {
    /**
     * @brief Dummy QWERTY layout mapping an Equipment ID (0-47) to its ASCII character.
     * Equipment IDs out of bounds return a null character.
     */
    constexpr std::array<char, 48> idToChar = {
        'q', 'w', 'e', 'r', 't', 'y',   'u', 'i', 'o', 'p', '[', '\\',
        'a', 's', 'd', 'f', 'g', 'h',   'j', 'k', 'l', ';', '\'', '\n',
        'z', 'x', 'c', 'v', 'b', 'n',   'm', ',', '.', '/', ' ', ' ',
        ' ', ' ', ' ', ' ', ' ', ' ',   ' ', ' ', ' ', ' ', ' ', ' ' // Thumbs and unused slots
    };
    //uiopuuiop

    /**
     * @brief Safely fetches the character associated with a specific Equipment ID.
     * @param equipmentId The 0-255 identifier of the hardware.
     * @return char The mapped character, or '\0' if out of bounds.
     */
    constexpr char getChar(uint8_t equipmentId) {
        if (equipmentId < idToChar.size()) {
            return idToChar[equipmentId];
        }
        return '\0';
    }
}