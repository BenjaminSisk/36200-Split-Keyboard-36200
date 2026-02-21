#pragma once

typedef enum error
{
    PIO_INIT_ERROR,
    DMA_INIT_ERROR,
    // TODO: IMPLEMENT MORE ERROR CODES
} error_code;

typedef enum mode
{
    BREATHING,
    COMET,
    RAINBOW_CYCLE,
    TRAVELING_RAINBOW,
    RIPPLE,
    COLUMN_FLASH,
    HEAT_MAP,
    SNAKE,
} mode_type;