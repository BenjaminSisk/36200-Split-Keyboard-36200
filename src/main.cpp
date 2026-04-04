#include "main.hpp"

#define DATAPIN0 25
#define SPACING 10
#define LEFT_OR_RIGHT 0 // left = 0, right = 1

#if LEFT_OR_RIGHT == 0
static auto key2led = Key2LED::loadFromJson("left_keymap.json");
#else
static auto key2led = Key2LED::loadFromJson("right_keymap.json");
#endif

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
            uint8_t id = (key_press >> 8) & 0xff;
            strips[0].x = id % 6;
            strips[0].y = id / 4;
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
    sleep_ms(500);
    multicore_launch_core1(main1);

    adc_init();
    init_uart_pins();
    init_uart_isr();

    printf("\033[2J");
    printf("System Initialized. Starting IRQs...\n");

    // Initialize using constructor delegation defaults
    // PicoJoystick joystick;
    // joystick.init(true);
    InputHandler inputHandler(hardwareMap::IS_LEFT_HALF);
    inputHandler.init();

    // The main loop is now entirely decoupled from sensor polling latency
    while (true)
    {

        inputHandler.update(); // Flushes the event queue to the FIFO, non-blocking
        // Output visualization using the filtered data
        // joystick.debugPrintSingleLine();

        inputHandler.debugPrint(); // Unified debug print for all peripherals

        // This sleep no longer blocks sensor reading!
        sleep_ms(100);
    }

    return 0;
}