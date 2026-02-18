#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <algorithm>
#include "pico/time.h"

#include "led.hpp"

#define NUM_STRIPS 4
#define LEDS_PER_STRIP 6
#define get_time_t() absolute_time_diff_us(t, get_absolute_time()) / 1000000.0f

typedef struct
{
    uint8_t r, g, b;
} RGB;

RGB leds[NUM_STRIPS][LEDS_PER_STRIP];

void init_led();

float get_hue(RGB leds[NUM_STRIPS][LEDS_PER_STRIP], int i, int j);

void hue_to_rgb(float hue, int i, int j);

/// @brief sine wave
/// @param leds
/// @param t
/// @param target_color
void pattern_sine_glow(RGB leds[NUM_STRIPS][LEDS_PER_STRIP], float t);

/// @brief linear comet
/// @param leds
/// @param t
/// @param decay
/// @param color
void pattern_comet_linear(RGB leds[NUM_STRIPS][LEDS_PER_STRIP], float t, float decay, RGB color);

/// @brief rainbow cycle
/// @param leds
/// @param t
/// @param spatial_freq 0 for 1 color for all keys, 255/LEDS_PER_STRIP to get all colors
void pattern_rainbow_linear(RGB leds[4][6], float t, float spatial_freq);

/// @brief traveling wave
/// @param leds
/// @param t
/// @param L
/// @param omega
void pattern_traveling_wave(RGB leds[NUM_STRIPS][LEDS_PER_STRIP], float t, float L, float omega);

///

/// @brief
/// @param leds
/// @param t
/// @param x
/// @param y
void pattern_ripple(RGB leds[NUM_STRIPS][LEDS_PER_STRIP], float t, int x, int y);

/// @brief
/// @param leds
/// @param t
/// @param x
/// @param y
void pattern_column_flash(RGB leds[NUM_STRIPS][LEDS_PER_STRIP], float t, int x, int y);

/// @brief
/// @param leds
/// @param t
/// @param x
/// @param y
void pattern_heat_map(RGB leds[NUM_STRIPS][LEDS_PER_STRIP], float t, int x, int y);

/// @brief
/// @param leds
/// @param t
/// @param x
/// @param y
void pattern_snake(RGB leds[NUM_STRIPS][LEDS_PER_STRIP], float t, int x, int y);
