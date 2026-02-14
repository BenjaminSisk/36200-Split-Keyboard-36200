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

    // set up shift left in FIFO (MSB first)
    // shifting only 24 bits skips first 8 MSB bits
    sm_config_set_out_shift(&c, false, true, 24);
    sm_config_set_sideset_pins(&c, data_pin);

    // connect gpio with pio
    pio_gpio_init(pio, data_pin);
    pio_sm_set_consecutive_pindirs(pio, state_machine, data_pin, 1, true);

    pio_sm_set_clkdiv(pio, state_machine, clock_get_hz(clk_sys) / 8000000);

    // initialize config
    pio_sm_init(pio, state_machine, offset, &c);

    pio_sm_set_enabled(pio, state_machine, true);
}

void led_strip::init_timer()
{
    timer0_hw->inte = (1 << 0);

    irq_set_exclusive_handler(timer_hardware_alarm_get_irq_num(timer_hw, 0), led_strip::output_wrapper);
    irq_set_enabled(timer_hardware_alarm_get_irq_num(timer_hw, 0), true);
    // latch for 10ms
    timer_hw->alarm[0] = (uint32_t)(timer_hw->timerawl + 10000);
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
    channel_config_set_dreq(&c, pio_get_dreq(pio, state_machine, true));

    dma_channel_configure(
        dma_channel,
        &c,
        &pio->txf[0],   // pio fifo to output
        rgb,            // buffer to read from
        LEDS_PER_STRIP, // 20 pixels
        false);
    dma_channel_start(dma_channel);
}

uint32_t led_strip::make_grb(uint8_t r, uint8_t g, uint8_t b)
{
    // will take format 0xXXGGRRBB where XX will not be used in PIO
    return ((uint32_t)g << 24) | ((uint32_t)r << 16) | ((uint32_t)b << 8);
}

void led_strip::put_pixel(uint32_t packet, int index)
{
    rgb[index] = packet;
}

void led_strip::output_strips_dma()
{
    timer_hw->intr = 1 << 0;

    // reset timer
    timer_hw->alarm[0] = timer_hw->timerawl + 20000;

    // restarts dma transfer
    dma_channel_set_read_addr(dma_channel, rgb, true);

    dma_channel_wait_for_finish_blocking(dma_channel);
}

void led_strip::output_wrapper()
{
    if (instance)
    {
        instance->output_strips_dma();
    }
}