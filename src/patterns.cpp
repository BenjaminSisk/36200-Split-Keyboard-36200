#include "patterns.hpp"

void init_led()
{
    for (int i = 0; i < NUM_STRIPS; i++)
    {
        for (int j = 0; j < LEDS_PER_STRIP; j++)
        {
            leds[i][j] = {
                .r = 0,
                .g = 0,
                .b = 0};
        }
    }
}

float get_hue(RGB leds[NUM_STRIPS][LEDS_PER_STRIP], int i, int j)
{
    float r_p = leds[i][j].r / 255.0f;
    float g_p = leds[i][j].g / 255.0f;
    float b_p = leds[i][j].b / 255.0f;

    float max_val = fmax(fmax(r_p, g_p), b_p);
    float min_val = fmin(fmin(r_p, g_p), b_p);

    float del = max_val - min_val;

    float hue = 0;
    if (max_val - r_p < 0.01)
    {
        hue = (g_p - b_p) / del * 60;
    }
    else if (max_val - g_p < 0.01)
    {
        hue = (2 + (b_p - r_p) / del) * 60;
    }
    else if (max_val - b_p < 0.01)
    {
        hue = (4 + (r_p - g_p) / del) * 60;
    }

    if (hue < 0)
        hue += 360;

    return hue;
}

void hue_to_rgb(float hue, int i, int j)
{
    if (hue < 85)
    {
        leds[i][j].r = 255 - 3 * hue;
        leds[i][j].g = 3 * hue;
        leds[i][j].b = 0;
    }
    else if (hue < 170)
    {
        leds[i][j].r = 0;
        leds[i][j].g = 255 - (hue - 85) * 3;
        leds[i][j].b = (hue - 85) * 3;
    }
    else
    {
        leds[i][j].r = (hue - 170) * 3;
        leds[i][j].g = 0;
        leds[i][j].b = 255 - (hue - 170) * 3;
    }
}

// TODO: IMPLEMENT PATTERNS
void pattern_sine_glow(RGB leds[NUM_STRIPS][LEDS_PER_STRIP], float t)
{
    for (int i = 0; i < NUM_STRIPS; i++)
    {
        for (int j = 0; j < LEDS_PER_STRIP; j++)
        {
            leds[NUM_STRIPS][LEDS_PER_STRIP].r *= (sin(t + j * 0.5) + 1) / 2;
            leds[NUM_STRIPS][LEDS_PER_STRIP].g *= (sin(t + j * 0.5) + 1) / 2;
            leds[NUM_STRIPS][LEDS_PER_STRIP].b *= (sin(t + j * 0.5) + 1) / 2;
        }
    }
}

void pattern_comet_linear(RGB leds[NUM_STRIPS][LEDS_PER_STRIP], float t, float decay, RGB color)
{
    for (int s = 0; s < NUM_STRIPS; s++)
    {
        for (int p = 0; p < LEDS_PER_STRIP; p++)
        {
            leds[s][p].r *= decay;
            leds[s][p].g *= decay;
            leds[s][p].b *= decay;
        }
    }

    // calculate head
    int headIndex = (int)t % (NUM_STRIPS * LEDS_PER_STRIP);

    // flatten head to 2d grid
    int s = headIndex / LEDS_PER_STRIP;
    int p = headIndex % LEDS_PER_STRIP;

    // set next head
    leds[s][p].r = color.r;
    leds[s][p].g = color.g;
    leds[s][p].b = color.b;
}

// spatial freq goes from 0 (1 solid changing color) to 255/LEDS_PER_STRIP
void pattern_rainbow_linear(RGB leds[4][6], float t, float spatial_freq)
{
    for (int i = 0; i < NUM_STRIPS; i++)
    {
        for (int j = 0; j < LEDS_PER_STRIP; j++)
        {
            // Formula: Hue = (Time_Offset + (Position * Density)) % 256
            uint8_t hue = (uint8_t)(t + (j * spatial_freq) + 10 * i) % 256;

            hue_to_rgb(hue, i, j);
        }
    }
}

void pattern_traveling_wave(RGB leds[NUM_STRIPS][LEDS_PER_STRIP], float t, float L, float omega)
{
    for (int i = 0; i < NUM_STRIPS; i++)
    {
        for (int j = 0; j < LEDS_PER_STRIP; j++)
        {
            leds[i][j].r = 255 * (sin(2 * M_PI * j / L - omega * (t + 0.5 * i)) + 1) / 2;
            leds[i][j].g = 255 * (sin(2 * M_PI * j / L - omega * (t + 0.5 * i)) + 1) / 2;
            leds[i][j].b = 255 * (sin(2 * M_PI * j / L - omega * (t + 0.5 * i)) + 1) / 2;
        }
    }
}
