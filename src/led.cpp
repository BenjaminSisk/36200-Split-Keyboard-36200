#include "led.hpp"

void led_strip::init_pio()
{
    offset = pio_add_program(pio, &ws2812_program);
    if (offset < 0)
    {
        printf("error: %d\n", PIO_INIT_ERROR);
        return;
    }
    // set default config based on pio
    pio_sm_config c = ws2812_program_get_default_config(offset);
    // set output gpio
    sm_config_set_out_pins(&c, data_pin, 1);
    pio_gpio_init(pio, data_pin);
    pio_sm_set_consecutive_pindirs(pio, state_machine, data_pin, 1, true);

    // initialize config
    pio_sm_init(pio, state_machine, offset, &c);

    pio_sm_set_enabled(pio, state_machine, true);
}

void led_strip::init_timer()
{
    timer0_hw->inte = (1 << 0);

    irq_set_exclusive_handler(timer_hardware_alarm_get_irq_num(timer_hw, 0), led_strip::output_wrapper);
    irq_set_enabled(timer_hardware_alarm_get_irq_num(timer_hw, 0), true);
    timer_hw->alarm[0] = (uint32_t)(timer_hw->timerawl + 1000);
}

void led_strip::init_dma()
{
    dma_channel_config c = dma_channel_get_default_config(dma_channel);

    // transfer uint32 rgb packets
    channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
    // want to read through array
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, false);

    // configs dma to send data to pio fifo
    channel_config_set_dreq(&c, pio_get_dreq(pio, 0, true));

    dma_channel_configure(
        dma_channel,
        &c,
        &pio->txf[0], // pio fifo to output
        rgb,          // buffer to read from
        20,           // 20 pixels
        false);
}

// TODO: IMPLEMENT VARIOUS PATTERNS

uint32_t led_strip::make_rgb(uint8_t r, uint8_t g, uint8_t b)
{
}

void led_strip::put_pixel()
{
}

void led_strip::output_strips_dma()
{
    timer_hw->intr = 1 << 0;

    // restarts dma transfer
    dma_channel_set_read_addr(dma_channel, rgb, true);
}

void led_strip::output_wrapper()
{
    if (instance)
    {
        instance->output_strips_dma();
    }
}