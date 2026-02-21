#include "main.hpp"

#define DATAPIN0 25
#define SPACING 10

void main1()
{
    init_led();
    led_strip strips[NUM_STRIPS];
    printf("Starting initialization...\n");
    for (int i = 0; i < NUM_STRIPS; i++)
    {
        int dma_chan = dma_claim_unused_channel(true);
        strips[i].init(DATAPIN0 + i, SPACING, i, pio0, i, dma_chan);

        strips[i].set_instance(i, &(strips[i]));
        printf("Instance pointer set to: %p\n", (void *)&(strips[i]));

        strips[i].set_base_color(255, 255, 255);
        strips[i].set_pattern_mode(RAINBOW_CYCLE);

        strips[i].init_pio();
        strips[i].init_dma();
        strips[i].init_timer();
    }

    printf("Initialization complete. Entering loop.\n");

    absolute_time_t target;

    while (true)
    {
        strips[0].update();
        target = delayed_by_us(get_absolute_time(), 100);
        while (absolute_time_diff_us(get_absolute_time(), target) > 0)
        {
            tight_loop_contents();
        }

        // strip.set_pattern_mode(COMET);
        // target = delayed_by_ms(get_absolute_time(), 5000);
        // while (absolute_time_diff_us(get_absolute_time(), target) > 0)
        // {
        //     // Do nothing, just keep the CPU cycling
        //     tight_loop_contents();
        // }

        // strip.set_pattern_mode(RAINBOW_CYCLE);
        // target = delayed_by_ms(get_absolute_time(), 10000);
        // while (absolute_time_diff_us(get_absolute_time(), target) > 0)
        // {
        //     // Do nothing, just keep the CPU cycling
        //     tight_loop_contents();
        // }

        // strip.set_pattern_mode(TRAVELING_RAINBOW);
        // target = delayed_by_ms(get_absolute_time(), 10000);
        // while (absolute_time_diff_us(get_absolute_time(), target) > 0)
        // {
        //     // Do nothing, just keep the CPU cycling
        //     tight_loop_contents();
        // }

        // strip.set_pattern_mode(RIPPLE);
        // target = delayed_by_ms(get_absolute_time(), 1000);
        // while (absolute_time_diff_us(get_absolute_time(), target) > 0)
        // {
        //     // Do nothing, just keep the CPU cycling
        //     tight_loop_contents();
        // }

        // strip.set_pattern_mode(COLUMN_FLASH);
        // target = delayed_by_ms(get_absolute_time(), 1000);
        // while (absolute_time_diff_us(get_absolute_time(), target) > 0)
        // {
        //     // Do nothing, just keep the CPU cycling
        //     tight_loop_contents();
        // }
    }
}

extern "C" int main()
{
    stdio_init_all();

    sleep_ms(500);

    multicore_launch_core1(main1);

    return 0;
}