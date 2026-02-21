#include "led.hpp"

led_strip *led_strip::instance = nullptr;

void led_strip::init_pio()
{
    offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, state_machine, offset, data_pin, 800000, false);

    printf("success: PIO initialized using helper\n");
}

void led_strip::init_timer()
{
    int irq_num = timer_hardware_alarm_get_irq_num(timer_hw, 0);
    if (irq_num < 0)
    {
        printf("error: Failed to get IRQ for timer alarm 0\n");
        return;
    }

    timer0_hw->inte = (1 << 0);

    irq_set_exclusive_handler(irq_num, led_strip::output_wrapper);
    irq_set_enabled(irq_num, true);
    // latch for 20ms
    timer_hw->alarm[0] = (uint32_t)(timer_hw->timerawl + 2000000);
    printf("success: Timer initialized\n");
}

void led_strip::init_dma()
{
    if (!dma_channel_is_claimed(dma_channel))
    {
        printf("warning: DMA channel %d is not claimed\n", dma_channel);
    }

    dma_channel_config c = dma_channel_get_default_config(dma_channel);

    // transfer uint32 rgb packets
    channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
    // want to read through array
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, false);

    // configs dma to send data to pio fifo
    channel_config_set_dreq(&c, pio_get_dreq(pio, state_machine, true));

    dma_channel_configure(
        dma_channel,
        &c,
        &pio->txf[state_machine], // pio fifo to output
        rgb,                      // buffer to read from
        LEDS_PER_STRIP,
        false);

    printf("success: DMA channel %d configured for PIO SM %d\n", dma_channel, state_machine);
}

uint32_t led_strip::make_grb(uint8_t r, uint8_t g, uint8_t b)
{
    // will take format 0xGGRRBBXX where XX will not be used in PIO
    return ((uint32_t)g << 24) | ((uint32_t)r << 16) | ((uint32_t)b << 8);
}

// MAYBE MAKE THIS DMA?
void led_strip::load_buffers()
{
    for (int j = 0; j < LEDS_PER_STRIP; j++)
    {
        rgb[j] = make_grb(leds[strip_num][j].r, leds[strip_num][j].g, leds[strip_num][j].b);
    }
}

void led_strip::output_strips_dma()
{
    timer_hw->intr = 1 << 0;

    if (dma_channel_is_busy(dma_channel))
    {
        printf("busy dma channel\n");
    }

    // calculate new RGB values
    absolute_time_t now = get_absolute_time();
    float t = to_us_since_boot(now) / 500000.0f; // change the divisor to change rate at which the LEDs change

    // IMPLEMENT THIS
    switch (mode)
    {
        // breathing
    case BREATHING:
        pattern_sine_glow(leds, t, r, g, b);
        break;
        // comet
    case COMET:
        pattern_comet_linear(leds, t, 0.8, r, g, b);
        break;
        // rainbow cycle
    case RAINBOW_CYCLE:
        pattern_rainbow_linear(leds, t, 255.0 / LEDS_PER_STRIP);
        break;
        // traveling sine wave
    case TRAVELING_RAINBOW:
        pattern_traveling_wave(leds, t, 48, 2);
        break;

        // ripple
    case RIPPLE:
        pattern_ripple(leds, t, 0.8, 2, 3);
        break;
        // column flash
    case COLUMN_FLASH:
        pattern_column_flash(leds, t, 2, 3);
        break;
        // heat map
    case HEAT_MAP:
        break;
        // snake
    case SNAKE:
        break;
    default:
        break;
    }

    // load RGB values into strip buffer (is this necessary? idk)
    load_buffers();

    // restarts dma transfer from buffer to pio
    dma_channel_set_read_addr(dma_channel, rgb, true);

    // reset timer
    timer_hw->alarm[0] = timer_hw->timerawl + 2000000;
}

void led_strip::output_wrapper()
{
    if (instance)
    {
        instance->output_strips_dma();
    }
    else
    {
        // Static error check to ensure class is linked
        static bool err_shown = false;
        if (!err_shown)
        {
            printf("error: output_wrapper called but instance is null\n");
            err_shown = true;
        }
    }
}

void led_strip::set_instance(led_strip *in_instance)
{
    instance = in_instance;
}

void led_strip::set_pattern_mode(mode_type new_mode)
{
    mode = new_mode;
}

void led_strip::set_base_color(int r0, int g0, int b0)
{
    r = r0;
    g = g0;
    b = b0;
}