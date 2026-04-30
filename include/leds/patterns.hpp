#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <algorithm>
#include <random>
#include "pico/time.h"

#include "led.hpp"

#define NUM_STRIPS 4
#define LEDS_PER_STRIP 6
#define get_time_t() absolute_time_diff_us(t, get_absolute_time()) / 1000000.0f

typedef struct _RGB
{
    uint8_t r, g, b;
} RGB;

extern RGB leds[NUM_STRIPS][LEDS_PER_STRIP];

void init_led();

float get_hue(int i, int j);

void hue_to_rgb(float hue, int i, int j);

/// @brief single color sine wave
/// @param t
/// @param target_color
void pattern_sine_glow(float t, int r, int g, int b);

/// @brief single color linear comet
/// @param t
/// @param decay
/// @param color
void pattern_comet_linear(float t, float decay, int r, int g, int b);

/// @brief all led rainbow cycle
/// @param t
/// @param spatial_freq 0 for 1 color for all keys, 255/LEDS_PER_STRIP to get all colors
void pattern_rainbow_linear(float t, float spatial_freq);

/// @brief traveling rainbow wave
/// @param t
/// @param L
/// @param omega
void pattern_traveling_wave(float t, float L, float omega);

///

/// @brief rainbow ripple on key press
/// @param t time since key press
/// @param x
/// @param y
void pattern_ripple(float t, float ripple_time, float decay_val, int x, int y);

/// @brief rainbow column flash on key press
/// @param t
/// @param x
/// @param y
void pattern_column_flash(float t, int x, int y);

/// @brief key changes value as key pressed more
/// @param t
/// @param x
/// @param y
void pattern_heat_map(int x, int y);

/// @brief random LEDs turn on on button press
/// @param t
/// @param x
/// @param y
void pattern_christmas(int x, int y);

/// @brief
/// @param t
/// @param x
/// @param y
void pattern_glitch_rain(float t, int x, int y);

/// @brief
/// @param t
/// @param x
/// @param y
void pattern_heat_wave(float t, int x, int y);
