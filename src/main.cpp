// #include "main.hpp"

// extern "C" int main()
// {
//     stdio_init_all();
//     gpio_init(11);
//     gpio_set_dir(11, true);

//     sleep_ms(2000);

//     printf("Starting initialization...\n");
//     int dma_chan = dma_claim_unused_channel(true);
//     led_strip strip(11, 10, 0, pio0, dma_chan);

//     led_strip::set_instance(&strip);
//     printf("Instance pointer set to: %p\n", (void *)&strip);

//     init_led();
//     strip.set_base_color(255, 255, 255);
//     strip.set_pattern_mode(BREATHING);

//     strip.init_pio();
//     strip.init_dma();

//     strip.init_timer();

//     printf("Initialization complete. Entering loop.\n");

//     dma_start_channel_mask((1 << dma_chan));
//     printf("started channel\n");

//     pio_sm_put_blocking(pio0, 0, 0xff000000);
//     printf("pio done\n");

//     for (int i = 0; i < LEDS_PER_STRIP; i++)
//     {
//         leds[0][i].r = 255;
//         leds[0][i].g = 0;
//         leds[0][i].b = 0;
//     }

//     strip.load_buffers();

//     dma_channel_set_read_addr(dma_chan, strip.rgb, true);
//     sleep_ms(2000);
//     while (true)
//     {
//         tight_loop_contents();
//     }

//     return 0;
// }

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "example_ws2812.pio.h"

#define IS_RGBW false
#define NUM_LEDS 6
#define DATA_PIN 11

extern "C" int main()
{
    stdio_init_all();

    // 1. Initialize the PIO
    PIO pio = pio0;
    uint state_machine = 0;
    uint offset = pio_add_program(pio, &ws2812_program);

    // 2. Use the helper function provided by your .pio.h file
    // Note: 800000 is the 800kHz frequency WS2812 expects
    ws2812_program_init(pio, state_machine, offset, DATA_PIN, 800000, IS_RGBW);

    while (true)
    {
        for (int i = 0; i < NUM_LEDS; i++)
        {
            // WS2812 expects GRB order.
            // Format: 0xGGRRBB00
            // Since the PIO shifts out 24 bits, we put Red in the middle byte
            // and shift it left so Red is the second 8-bit block sent.
            uint32_t red_pixel = (0x00 << 24) | (0xFF << 16) | (0x00 << 8);

            pio_sm_put(pio, state_machine, red_pixel);
        }

        // Wait 1 second before refreshing (WS2812 stays lit until updated)
        sleep_ms(1000);
    }
}