#include "main.hpp"

#define DATAPIN0 25
#define SPACING 10

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
        strips[i].set_pattern_mode(COLUMN_FLASH);
        strips[i].init_pio();
        strips[i].init_dma();
        strips[i].init_timer();
    }

    printf("Initialization complete. Entering loop.\n");

    absolute_time_t target;
    float press_time = 0;
    float prev_time = 0;

    mode current_mode = BREATHING;
    int r_color = 0;
    int g_color = 255;
    int b_color = 0;

    while (true)
    {
        absolute_time_t now = get_absolute_time();
        float t = to_us_since_boot(now) / 50000.0f;

        while (t - prev_time < 200)
        {
            now = get_absolute_time();
            t = to_us_since_boot(now) / 50000.0f;
            strips[0].x = -1;
            strips[0].y = -1;
            uint8_t action = -1;
            if (multicore_fifo_rvalid())
            {
                int temp = 0;
                uint32_t key_press = multicore_fifo_pop_blocking();
                uint8_t id = (key_press >> 8) & 0xff;
                action = (key_press) & 0xff;
                if (!hardwareMap::IS_LEFT_HALF)
                    temp = 6;
                if (action == 0x01)
                {
                    strips[0].x = id % 12 - temp;
                    strips[0].y = id / 12;
                }
                press_time = t;
            }
            strips[0].update(press_time, t);
            target = delayed_by_us(get_absolute_time(), 5000);
            while (absolute_time_diff_us(get_absolute_time(), target) > 0)
            {
                tight_loop_contents();
            }
        }

        prev_time = t;
        current_mode = (mode)((current_mode + 1) % 10);
        r_color = (r_color + 40) % 256;
        g_color = (g_color + 70) % 256;
        b_color = (b_color + 100) % 256;

        for (int i = 0; i < NUM_STRIPS; i++)
        {
            strips[i].set_pattern_mode(current_mode);
            strips[i].set_base_color(r_color, g_color, b_color);
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
    g_inputHandler = &inputHandler;

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