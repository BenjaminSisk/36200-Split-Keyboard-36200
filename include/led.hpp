#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "pico/time.h"

#define FRAC_BITS 4
#define LEDS_PER_STRIP 6
#define WS2812_PIN_BASE 2

#if WS2812_PIN_BASE >= NUM_BANK0_GPIOS
#error Attempting to use a pin>=32 on a platform that does not support it
#endif

#include "example_ws2812.pio.h"
#include "error_codes.hpp"

class led_strip
{
public:
    led_strip(int in_pwr, int in_gnd, int in_data, int in_spacing)
    {
        pwr_pin = in_pwr;
        gnd_pin = in_gnd;
        data_pin = in_data;
        spacing = in_spacing;
    };

    /// @brief init pio peripherial
    void init_pio();
    // note: led usually runs at 800kHz. Sysclk = 150MHz and requires 10 cycles/bit so need a clk divider of 150/8

    /// @brief timer for data latch
    void init_timer();

    /// @brief initializes dma for pio
    void init_dma();

    /// @brief converts r,g,b bytes into single 32 bit format
    /// @param r
    /// @param g
    /// @param b
    /// @return 32 package for ws2812
    uint32_t make_grb(uint8_t r, uint8_t g, uint8_t b);

    /// @brief writes rgb into strip buffer
    /// @param rgb
    /// @param index
    void put_pixel(uint32_t rgb, int index);

    /// @brief updates memory pointer for next frame (output every 1ms)
    void output_strips_dma();

    /// @brief wrapper for irq for outputting ws2812
    static void output_wrapper();

    PIO pio;
    uint offset;
    int state_machine;
    int dma_channel;

    uint8_t r, g, b;
    uint8_t spacing;

private:
    static led_strip *instance;

    int pwr_pin;
    int gnd_pin;
    int data_pin;

    uint32_t rgb[LEDS_PER_STRIP];
};
