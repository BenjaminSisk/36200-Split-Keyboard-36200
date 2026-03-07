#pragma once
#include <vector>
#include <cstdint>

class KeyMap {
public:
    // Pass in the 1D representation of your keypad's characters
    KeyMap(const std::vector<char>& layoutMap);

    // Safe getter that returns the character, or '\0' if index is out of bounds
    char getChar(uint8_t buttonIndex) const;

    // Optional: Allow swapping the layout at runtime
    void setLayout(const std::vector<char>& newLayout);

private:
    std::vector<char> layout;
};