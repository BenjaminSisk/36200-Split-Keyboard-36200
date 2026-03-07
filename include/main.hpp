#include "pico/multicore.h"
#include <map>
#include <vector>
#include <random>

#include "led.hpp"
#include "patterns.hpp"
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "PicoJoystick.h"
#include "uart_comm.h"
#include "key2led.hpp"

/// @brief main core 1
void main1();

/// @brief main core 0
/// @return 0
int main();