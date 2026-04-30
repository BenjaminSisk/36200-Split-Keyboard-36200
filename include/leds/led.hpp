#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "pico/time.h"

#include "patterns.hpp"

#define NUM_STRIPS 4
#define LEDS_PER_STRIP 6
#define WS2812_PIN_BASE 2

#if WS2812_PIN_BASE >= NUM_BANK0_GPIOS
#error Attempting to use a pin>=32 on a platform that does not support it
#endif

#include "example_ws2812.pio.h"
#include "error_codes.hpp"

typedef enum mode
{
    BREATHING = 0,
    COMET = 1,
    RAINBOW_CYCLE = 2,
    TRAVELING_RAINBOW = 3,
    RIPPLE = 4,
    COLUMN_FLASH = 5,
    HEAT_MAP = 6,
    CHRISTMAS = 7,
    GLITCH = 8,
    HEAT_WAVE = 9,
} mode_type;

class led_strip
{
public:
    led_strip() {};

    /// @brief init pins
    void init(int in_data, int in_spacing, int in_num, PIO in_pio, int in_sm, int in_dma_chan);

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

    /// @brief converts rgb struct into uint32 and loads into buffer
    /// @param leds
    /// @param rgb
    void load_buffers();

    /// @brief calculates new RGB values from pattern and loads into buffer
    void update(float ripple_time, float t);

    /// @brief sends led strip buffer to pio to output
    void output_strip();

    /// @brief sends led strip buffer to pio to output with dma
    void output_strips_dma();

    /// @brief wrapper for irq for outputting ws2812
    static void output_wrapper();

    /// @brief sets the pattern
    void set_pattern_mode(mode_type new_mode);

    /// @brief sets base rgb values
    /// @param r
    /// @param g
    /// @param b
    void set_base_color(int r, int g, int b);

    /// @brief inits instance
    /// @param in_instance
    static void set_instance(int index, led_strip *in_instance);

    static int x;
    static int y;

private:
    static led_strip *instances[NUM_STRIPS];

    int data_pin;

    uint32_t rgb[LEDS_PER_STRIP];

    // pio number
    PIO pio;
    uint offset;
    // state machine number
    int state_machine;
    int dma_channel;

    // spacing used to calculate patterns
    uint8_t spacing;

    // led strip number
    int strip_num;

    // pattern mode
    mode_type mode = BREATHING;

    // base colors
    int r = 0;
    int g = 0;
    int b = 0;
};
