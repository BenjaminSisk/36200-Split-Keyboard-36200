#include "led.hpp"

led_strip *led_strip::instances[NUM_STRIPS] = {nullptr};

void led_strip::init(int in_data, int in_spacing, int in_num, PIO in_pio, int in_sm, int in_dma_chan)
{
    data_pin = in_data;
    spacing = in_spacing;
    strip_num = in_num;
    pio = in_pio;
    dma_channel = in_dma_chan;
    state_machine = in_sm;

    instances[in_num] = this;
}

void led_strip::init_pio()
{
    offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, state_machine, offset, data_pin, 800000, false);

    printf("error_code: %d\n", SUCCESS);
}

void led_strip::init_timer()
{
    int irq_num = timer_hardware_alarm_get_irq_num(timer_hw, 0);
    if (irq_num < 0)
    {
        printf("error_code: %d\n", TIMER_INIT_ERROR);
        return;
    }

    timer0_hw->inte = (1 << 0);

    irq_set_exclusive_handler(irq_num, led_strip::output_wrapper);
    irq_set_enabled(irq_num, true);
    timer_hw->alarm[0] = (uint32_t)(timer_hw->timerawl + 20000);
    printf("error_code: %d\n", SUCCESS);
}

void led_strip::init_dma()
{
    if (!dma_channel_is_claimed(dma_channel))
    {
        printf("error_code: %d\n", RESOURCE_CLAIM_ERROR);
    }

    dma_channel_config c = dma_channel_get_default_config(dma_channel);

    channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, false);

    channel_config_set_dreq(&c, pio_get_dreq(pio, state_machine, true));

    dma_channel_configure(
        dma_channel,
        &c,
        &pio->txf[state_machine],
        rgb,
        LEDS_PER_STRIP,
        false);

    printf("error_code: %d\n", SUCCESS);
}

uint32_t led_strip::make_grb(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)g << 24) | ((uint32_t)r << 16) | ((uint32_t)b << 8);
}

void led_strip::load_buffers()
{
    for (int j = 0; j < LEDS_PER_STRIP; j++)
    {
        rgb[j] = make_grb(leds[strip_num][j].r, leds[strip_num][j].g, leds[strip_num][j].b);
    }
}

void led_strip::update()
{
    absolute_time_t now = get_absolute_time();
    float t = to_us_since_boot(now) / 50000.0f;

    switch (mode)
    {
    case BREATHING:
        pattern_sine_glow(leds, t, r, g, b);
        break;
    case COMET:
        pattern_comet_linear(leds, t, 0.8, r, g, b);
        break;
    case RAINBOW_CYCLE:
        pattern_rainbow_linear(leds, t, 255.0 / LEDS_PER_STRIP);
        break;
    case TRAVELING_RAINBOW:
        pattern_traveling_wave(leds, t, 48, 2);
        break;
    case RIPPLE:
        pattern_ripple(leds, t, 0.8, 2, 3);
        break;
    case COLUMN_FLASH:
        pattern_column_flash(leds, t, 2, 3);
        break;
    case HEAT_MAP:
        break;
    case SNAKE:
        break;
    default:
        break;
    }
}

void led_strip::output_strip()
{
    timer_hw->intr = 1 << 0;

    load_buffers();

    for (int i = 0; i < LEDS_PER_STRIP; i++)
        pio_sm_put(pio, state_machine, rgb[i]);

    timer_hw->alarm[0] = timer_hw->timerawl + 20000;
}

void led_strip::output_strips_dma()
{
    timer_hw->intr = 1 << 0;

    load_buffers();

    if (dma_channel_is_busy(dma_channel))
    {
        printf("error_code: %d\n", DMA_BUSY_TIMEOUT);
        return;
    }

    dma_channel_transfer_from_buffer_now(dma_channel, rgb, LEDS_PER_STRIP);

    timer_hw->alarm[0] = timer_hw->timerawl + 20000;
}

void led_strip::output_wrapper()
{
    for (int i = 0; i < NUM_STRIPS; i++)
    {
        if (instances[i])
        {
            instances[i]->output_strips_dma();
        }
        else
        {
            static bool err_shown = false;
            if (!err_shown)
            {
                printf("error_code: %d\n", NULL_INSTANCE_ERROR);
                err_shown = true;
            }
        }
    }
}

void led_strip::set_instance(int index, led_strip *in_instance)
{
    if (index < 0 || index >= NUM_STRIPS)
    {
        printf("error_code: %d\n", INVALID_STRIP_INDEX);
        return;
    }
    instances[index] = in_instance;
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