#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/sem.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/irq.h"

#define FRAC_BITS 4
#define NUM_PIXELS 64
#define WS2812_PIN_BASE 2

// Check the pin is compatible with the platform
#if WS2812_PIN_BASE >= NUM_BANK0_GPIOS
#error Attempting to use a pin>=32 on a platform that does not support it
#endif

#include "example_ws2812.pio.h" // generated header

// necessary functions
void put_pixel();
uint32_t make_rgb(uint8_t r, uint8_t g, uint8_t b);
