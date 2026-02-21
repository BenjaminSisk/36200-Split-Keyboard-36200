#include "main.hpp"

#define DATAPIN0 25
#define SPACING 10

struct Coord
{
    int row;
    int col;
};

static std::map<char, Coord> leftHandGrid = {
    // Row 0 (Numbers/Tilde)
    {'`', {0, 0}},
    {'1', {0, 1}},
    {'2', {0, 2}},
    {'3', {0, 3}},
    {'4', {0, 4}},
    {'5', {0, 5}},
    // Row 1 (Tab / Q-T)
    {'\t', {1, 0}},
    {'q', {1, 1}},
    {'w', {1, 2}},
    {'e', {1, 3}},
    {'r', {1, 4}},
    {'t', {1, 5}},
    // Row 2 (A-G)
    {'a', {2, 1}},
    {'s', {2, 2}},
    {'d', {2, 3}},
    {'f', {2, 4}},
    {'g', {2, 5}},
    // Row 3 (Z-B)
    {'z', {3, 1}},
    {'x', {3, 2}},
    {'c', {3, 3}},
    {'v', {3, 4}},
    {'b', {3, 5}}};

static std::map<char, Coord> rightHandGrid = {
    // Row 0: 6-0, -
    {'6', {0, 0}},
    {'7', {0, 1}},
    {'8', {0, 2}},
    {'9', {0, 3}},
    {'0', {0, 4}},
    {'-', {0, 5}},
    // Row 1: y, u, i, o, p, [
    {'y', {1, 0}},
    {'u', {1, 1}},
    {'i', {1, 2}},
    {'o', {1, 3}},
    {'p', {1, 4}},
    {'[', {1, 5}},
    // Row 2: h, j, k, l, ;, '
    {'h', {2, 0}},
    {'j', {2, 1}},
    {'k', {2, 2}},
    {'l', {2, 3}},
    {';', {2, 4}},
    {'\'', {2, 5}},
    // Row 3: n, m, ,, ., /, Enter
    {'n', {3, 0}},
    {'m', {3, 1}},
    {',', {3, 2}},
    {'.', {3, 3}},
    {'/', {3, 4}},
    {'\r', {3, 5}}};

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
        char key = (int)tolower(key_press);
        struct Coord pos = {-1, -1};
        if (leftHandGrid.count(key))
        {
            pos = leftHandGrid[key];
        }
        if (rightHandGrid.count(key))
        {
            pos = rightHandGrid[key];
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