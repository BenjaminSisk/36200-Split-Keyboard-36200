#include "patterns.hpp"

RGB leds[NUM_STRIPS][LEDS_PER_STRIP];

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
    if (del < 0.001)
        return 0;

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
void pattern_sine_glow(RGB leds[NUM_STRIPS][LEDS_PER_STRIP], float t, int r, int g, int b)
{
    for (int i = 0; i < NUM_STRIPS; i++)
    {
        for (int j = 0; j < LEDS_PER_STRIP; j++)
        {
            leds[i][j].r = r * (sin(t + j * 0.5) + 1) / 2;
            leds[i][j].g = g * (sin(t + j * 0.5) + 1) / 2;
            leds[i][j].b = b * (sin(t + j * 0.5) + 1) / 2;
        }
    }
}

// decay should be greater than 0.75
void pattern_comet_linear(RGB leds[NUM_STRIPS][LEDS_PER_STRIP], float t, float decay, int r, int g, int b)
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
    leds[s][p].r = r;
    leds[s][p].g = g;
    leds[s][p].b = b;
}

// spatial freq goes from 0 (1 solid changing color) to 255.0/LEDS_PER_STRIP
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

// rainbow wave
void pattern_traveling_wave(RGB leds[NUM_STRIPS][LEDS_PER_STRIP], float t, float L, float omega)
{
    for (int i = 0; i < NUM_STRIPS; i++)
    {
        for (int j = 0; j < LEDS_PER_STRIP; j++)
        {
            uint8_t hue = (uint8_t)(t + (j * 10)) % 256;

            hue_to_rgb(hue, i, j);

            leds[i][j].r *= (sin(2 * M_PI * j / L - omega * (t + 0.5 * i)) + 1) / 2;
            leds[i][j].g *= (sin(2 * M_PI * j / L - omega * (t + 0.5 * i)) + 1) / 2;
            leds[i][j].b *= (sin(2 * M_PI * j / L - omega * (t + 0.5 * i)) + 1) / 2;
        }
    }
}

//

// NEED TO ADJUST t
// (x,y) == (-1,-1) if no key press
void pattern_ripple(RGB leds[NUM_STRIPS][LEDS_PER_STRIP], float t, float decay, int x, int y)
{
    for (int i = 0; i < NUM_STRIPS; i++)
    {
        for (int j = 0; j < LEDS_PER_STRIP; j++)
        {
            // get hue for rainbow
            uint8_t hue = (uint8_t)(t + (j * 10)) % 256;

            hue_to_rgb(hue, i, j);

            // calculate distance from key press
            float distance = pow(pow(i - y, 2) + pow(j - x, 2), 0.5);

            // add decay the longer the key has not been pressed
            leds[i][j].r *= fmax(0, 255 - decay * fabs(distance - M_PI * t));
            leds[i][j].g *= fmax(0, 255 - decay * fabs(distance - M_PI * t));
            leds[i][j].b *= fmax(0, 255 - decay * fabs(distance - M_PI * t));
        }
    }
}

void pattern_column_flash(RGB leds[NUM_STRIPS][LEDS_PER_STRIP], float t, int x, int y)
{
    float fadeSpeed = 3.0;                  // Higher = faster fade
    float brightness = exp(-fadeSpeed * t); // Exponential decay (starts at 1.0)

    // stop processing if the flash is too dim to see
    if (brightness < 0.01)
        return;

    for (int i = 0; i < NUM_STRIPS; i++)
    {
        // The 'y' offset ensures the rainbow is centered around the press
        uint8_t hue = (uint8_t)((i * 40) + (t * 100) + (y * 20)) % 256;

        // Convert to RGB with the decay applied to the 'Value' (Brightness)
        hue_to_rgb(hue, i, x);
        leds[i][x].r *= brightness;
        leds[i][x].g *= brightness;
        leds[i][x].b *= brightness;
    }
}
