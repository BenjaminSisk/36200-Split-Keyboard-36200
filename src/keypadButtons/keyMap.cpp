/**
 * KeyMap.cpp
 * Implementation of the KeyMap class: a simple mapping of button indices to characters for a keypad.
 * This allows you to easily translate the raw button index from the KeypadButtons class into meaningful characters
 */

#include "KeyMap.h"

KeyMap::KeyMap(const std::vector<char>& layoutMap) : layout(layoutMap) {}

char KeyMap::getChar(uint8_t buttonIndex) const {
    if (buttonIndex < layout.size()) {
        return layout[buttonIndex];
    }
    return '\0'; // Return null char if the index is somehow out of bounds
}

void KeyMap::setLayout(const std::vector<char>& newLayout) {
    layout = newLayout;
}