#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <algorithm>
#include "pico/time.h"

#define NUM_STRIPS 4
#define LEDS_PER_STRIP 6
#define get_time_t() absolute_time_diff_us(start_time, get_absolute_time()) / 1000000.0f

typedef struct
{
    uint8_t r, g, b;
} RGB;

/// @brief breathing pattern
/// @param leds
/// @param t
void pattern_breathe(RGB leds[NUM_STRIPS][LEDS_PER_STRIP], float t);

/// @brief diagonal comet pattern
/// @param leds
/// @param t
void pattern_comet(RGB leds[NUM_STRIPS][LEDS_PER_STRIP], float t);

/// @brief rainbow pattern
/// @param leds
/// @param t
void pattern_rainbow(RGB leds[NUM_STRIPS][LEDS_PER_STRIP], float t);

/// @brief ripple pattern about key hit
/// @param leds
/// @param t
/// @param hit_x
/// @param hit_y
/// @param hit_time
void pattern_ripple(RGB leds[NUM_STRIPS][LEDS_PER_STRIP], float t, int hit_x, int hit_y, float hit_time);

/// @brief sine wave
/// @param leds
/// @param t
/// @param target_color
void pattern_sine_glow(RGB leds[NUM_STRIPS][LEDS_PER_STRIP], float t, RGB target_color);

/// @brief linear comet
/// @param leds
/// @param t
/// @param decay
/// @param color
void pattern_comet_linear(RGB leds[NUM_STRIPS][LEDS_PER_STRIP], float t, float decay, RGB color);

/// @brief rainbow cycle
/// @param leds
/// @param t
/// @param spacing
void pattern_rainbow_cycle(RGB leds[NUM_STRIPS][LEDS_PER_STRIP], float t, float spacing);

/// @brief traveling wave
/// @param leds
/// @param t
/// @param L
/// @param omega
/// @param color
void pattern_traveling_wave(RGB leds[NUM_STRIPS][LEDS_PER_STRIP], float t, float L, float omega, RGB color);
