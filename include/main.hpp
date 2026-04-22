#include <map>
#include <vector>
#include <random>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <quertyMap.h>

#include "patterns.hpp"
#include "uart_comm.h"
#include "hardware/adc.h"
#include "hardware/structs/resets.h"
#include "hardware/irq.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "pico/unique_id.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"

#include "inputHandler.h"
#include "led.hpp"
#include "key2led.hpp"
#include "usb.hpp"

/// @brief main core 1
void main1();

/// @brief main core 0
/// @return 0
int main();