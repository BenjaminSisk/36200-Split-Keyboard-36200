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
void put_pixel();                                   // writes rgb into strip buffer
uint32_t make_rgb(uint8_t r, uint8_t g, uint8_t b); // converts r,g,b bytes into 32 bit format for ws2812
void dma_init();                                    // init dma
void dma_handler();                                 // An Interrupt Service Routine (ISR). When the DMA finishes sending data, this starts a 400µs timer. This "quiet time" tells the WS2812 LEDs to "latch" the data and show the colors.
void output_strips_dma();                           // Updates the memory pointers for the next frame and triggers the DMA sequence.

/*
To drive multiple strips at once, the PIO needs one bit from every strip at the exact same time.
*/

void transform_strips(); // This converts standard 8-bit color values (0–255) into "bit planes." It calculates the brightness for each pixel and then "rotates" the data so that bit $N$ of all strips is stored together.
