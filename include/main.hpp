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
#include "keymap_reader.hpp"

/// @brief main core 1
void main1();

int main();