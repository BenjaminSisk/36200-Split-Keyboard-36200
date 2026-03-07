#include "main.hpp"

#define DATAPIN0 25
#define SPACING 10

struct Coord
{
    int row;
    int col;
};

static std::map<int, Coord> keymap = {
    // Row 0 (Numbers/Tilde)
    {0, {0, 0}},
    {1, {0, 1}},
    {2, {0, 2}},
    {3, {0, 3}},
    {4, {0, 4}},
    {5, {0, 5}},
    // Row 1 (Tab / Q-T)
    {6, {1, 0}},
    {7, {1, 1}},
    {8, {1, 2}},
    {9, {1, 3}},
    {10, {1, 4}},
    {11, {1, 5}},
    // Row 2 (A-G)
    {12, {2, 1}},
    {13, {2, 2}},
    {14, {2, 3}},
    {15, {2, 4}},
    {16, {2, 5}},
    // Row 3 (Z-B)
    {17, {3, 1}},
    {18, {3, 2}},
    {19, {3, 3}},
    {20, {3, 4}},
    {21, {3, 5}}};

void main1()
{
    stdio_init_all();
    init_led();
    led_strip strips[NUM_STRIPS];
    printf("Starting initialization...\n");
    for (int i = 0; i < NUM_STRIPS; i++)
    {
        int dma_chan = dma_claim_unused_channel(true);
        strips[i].init(DATAPIN0 + i, SPACING, i, pio0, i, dma_chan);

        strips[i].set_instance(i, &(strips[i]));
        printf("Instance pointer set to: %p\n", (void *)&(strips[i]));

        strips[i].set_base_color(0, 255, 0);
        strips[i].set_pattern_mode(COMET);

        strips[i].init_pio();
        strips[i].init_dma();
        strips[i].init_timer();
    }

    printf("Initialization complete. Entering loop.\n");

    absolute_time_t target;

    while (true)
    {
        if (multicore_fifo_rvalid())
        {

            uint32_t key_press = multicore_fifo_pop_blocking();
            struct Coord pos = {-1, -1};
            if (keymap.count(key_press))
            {
                pos = keymap[key_press];
            }
            strips[0].x = pos.col;
            strips[0].y = pos.row;
            // printf("read keypress: (%d, %d)\n", pos.row, pos.col);
        }
        strips[0].update();
        target = delayed_by_us(get_absolute_time(), 100);
        while (absolute_time_diff_us(get_absolute_time(), target) > 0)
        {
            tight_loop_contents();
        }
    }
}

int main()
{
    stdio_init_all();

    multicore_launch_core1(main1);

    adc_init();

    printf("\033[2J");
    printf("System Initialized. Starting IRQs...\n");

    // Initialize using constructor delegation defaults
    PicoJoystick joystick;
    joystick.init();

    // Fire up the hardware timer IRQ
    joystick.startTimer();

    // The main loop is now entirely decoupled from sensor polling latency
    while (true)
    {
        // Output visualization using the filtered data
        joystick.debugPrintSingleLine();

        // This sleep no longer blocks sensor reading!
        sleep_ms(100);
    }

    return 0;
}