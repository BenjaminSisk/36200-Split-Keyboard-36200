#include "pico/multicore.h"

#include "led.hpp"
#include "patterns.hpp"

/// @brief main core 0
/// @return
extern "C" int main();

/// @brief main core 1
void main1();