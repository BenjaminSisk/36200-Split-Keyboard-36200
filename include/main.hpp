#include "pico/multicore.h"
#include <map>
#include <vector>
#include <random>

#include "led.hpp"
#include "patterns.hpp"

/// @brief main core 0
/// @return
extern "C" int main();

/// @brief main core 1
extern "C" int main1();