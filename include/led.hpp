#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/irq.h"

#define FRAC_BITS 4
#define NUM_PIXELS 64
#define WS2812_PIN_BASE 2

#if WS2812_PIN_BASE >= NUM_BANK0_GPIOS
#error Attempting to use a pin>=32 on a platform that does not support it
#endif

#include "example_ws2812.pio.h"

typedef enum error
{
    PIO_INIT_ERROR,
    DMA_INIT_ERROR,
} error_code;

class led_strip
{
public:
    led_strip(int in_pwr, int in_gnd, int in_data)
    {
        pwr_pin = in_pwr;
        gnd_pin = in_gnd;
        data_pin = in_data;
    };

    /// @brief init pio peripherial
    void init_pio();

    /// @brief timer for data latch
    void init_timer();

    /// @brief initializes dma for pio
    void init_dma();

    // TODO: IMPLEMENT VARIOUS PATTERNS

    //

    /// @brief converts r,g,b bytes into single 32 bit format
    /// @param r
    /// @param g
    /// @param b
    /// @return 32 package for ws2812
    uint32_t make_rgb(uint8_t r, uint8_t g, uint8_t b);

    /// @brief writes rgb into strip buffer
    void put_pixel();

    /// @brief converts 8 bit color values (0-255) into bit planes. To drive multiple strips at once, the PIO needs one bit from every strip at the exact same time.
    void transform_strips();

    /// @brief updates memory pointer for next frame (output every 1ms)
    void output_strips_dma();

    /// @brief wrapper for irq for outputting ws2812
    static void output_wrapper();

    PIO pio;
    uint offset;
    int state_machine;
    int dma_channel;

private:
    static led_strip *instance;

    int pwr_pin;
    int gnd_pin;
    int data_pin;

    uint32_t rgb[20];
};
