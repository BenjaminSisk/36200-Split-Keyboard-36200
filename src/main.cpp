#include "main.hpp"

#define DATAPIN0 25
#define SPACING 10

struct Coord
{
    int row;
    int col;
};

static std::map<int, Coord> leftHandGrid = {
    // Row 0 (Numbers/Tilde)
    {96, {0, 0}},
    {49, {0, 1}},
    {50, {0, 2}},
    {51, {0, 3}},
    {52, {0, 4}},
    {53, {0, 5}},
    // Row 1 (Q-T)
    {9, {1, 0}},
    {113, {1, 1}},
    {119, {1, 2}},
    {101, {1, 3}},
    {114, {1, 4}},
    {116, {1, 5}},
    // Row 2 (A-G)
    {97, {2, 1}},
    {115, {2, 2}},
    {100, {2, 3}},
    {102, {2, 4}},
    {103, {2, 5}},
    // Row 3 (Z-B)
    {122, {3, 1}},
    {120, {3, 2}},
    {99, {3, 3}},
    {118, {3, 4}},
    {98, {3, 5}}};

static std::map<int, Coord> rightHandGrid = {
    // Row 0: 6, 7, 8, 9, 0, -
    {54, {0, 0}},
    {55, {0, 1}},
    {56, {0, 2}},
    {57, {0, 3}},
    {48, {0, 4}},
    {45, {0, 5}},
    // Row 1: y, u, i, o, p, [
    {121, {1, 0}},
    {117, {1, 1}},
    {105, {1, 2}},
    {111, {1, 3}},
    {112, {1, 4}},
    {91, {1, 5}},
    // Row 2: h, j, k, l, ;, '
    {104, {2, 0}},
    {106, {2, 1}},
    {107, {2, 2}},
    {108, {2, 3}},
    {59, {2, 4}},
    {39, {2, 5}},
    // Row 3: n, m, ,, ., /, Enter(approx)
    {110, {3, 0}},
    {109, {3, 1}},
    {44, {3, 2}},
    {46, {3, 3}},
    {47, {3, 4}},
    {13, {3, 5}}};

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
        uint32_t key_press = multicore_fifo_pop_blocking();
        key_press = (int)tolower(key_press);
        struct Coord pos;
        if (leftHandGrid.count(key_press))
        {
            pos = leftHandGrid[key_press];
        }
        if (rightHandGrid.count(key_press))
        {
            pos = rightHandGrid[key_press];
        }
        strips[0].x = pos.col;
        strips[0].y = pos.row;
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