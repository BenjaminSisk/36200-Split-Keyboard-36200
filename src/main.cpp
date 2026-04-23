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
            uint8_t action = (key_press) & 0xff;
            printf("id: %d, value: %d\n", id, action);
            struct Coord pos = {-1, -1};
            if (key2led.count(id))
            {
                pos = key2led[id];
            }
            strips[0].x = pos.col;
            strips[0].y = pos.row;
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
    
    board_init();
    tusb_rhport_init_t dev_init = {.role = TUSB_ROLE_DEVICE, .speed = TUSB_SPEED_AUTO};
    tusb_init(BOARD_TUD_RHPORT, &dev_init);
    board_init_after_tusb();

    adc_init();
    init_uart_pins();
    init_uart_isr();

    InputHandler inputHandler;
    inputHandler.init();
    g_inputHandler = &inputHandler;wowopwopwopwopwopwopwopwopwopwopwopwopwopwopwop

    // Let Core 1 finish its setup prints before we start spamming \r
    sleep_ms(1500);
    printf("\033[2J");
    printf("System Initialized. Starting main loop...\n");

    while (true)
    {
        inputHandler.update();
        inputHandler.debugPrint();
        // joystick.debugPrintSingleLine();




        tud_task();
        hid_task(inputHandler);
    } 
}