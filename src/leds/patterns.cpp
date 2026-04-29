#include "patterns.hpp"

RGB leds[NUM_STRIPS][LEDS_PER_STRIP];
float shadow_leds[NUM_STRIPS][LEDS_PER_STRIP][3];

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

            shadow_leds[i][j][0] = 0.000; // Red
            shadow_leds[i][j][1] = 0.000; // Green
            shadow_leds[i][j][2] = 0.000; // Blue
        }
    }
}

float get_hue(int i, int j)
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
        hue += 255;

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
void pattern_sine_glow(float t, int r, int g, int b)
{
    for (int i = 0; i < NUM_STRIPS; i++)
    {
        for (int j = 0; j < LEDS_PER_STRIP; j++)
        {
            leds[i][j].r = r * (sin(t / 10 + j * 0.5) + 1) / 2;
            leds[i][j].g = g * (sin(t / 10 + j * 0.5) + 1) / 2;
            leds[i][j].b = b * (sin(t / 10 + j * 0.5) + 1) / 2;
        }
    }
}

// decay should be greater than 0.75
void pattern_comet_linear(float t, float decay, int r, int g, int b)
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
void pattern_rainbow_linear(float t, float spatial_freq)
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
void pattern_traveling_wave(float t, float L, float omega)
{
    for (int i = 0; i < NUM_STRIPS; i++)
    {
        for (int j = 0; j < LEDS_PER_STRIP; j++)
        {
            uint8_t hue = (uint8_t)(t + (j * 10)) % 256;

            hue_to_rgb(hue, i, j);

            leds[i][j].r *= (sin(2 * M_PI * j / L - omega * (t / 10 + 0.5 * i)) + 1) / 2;
            leds[i][j].g *= (sin(2 * M_PI * j / L - omega * (t / 10 + 0.5 * i)) + 1) / 2;
            leds[i][j].b *= (sin(2 * M_PI * j / L - omega * (t / 10 + 0.5 * i)) + 1) / 2;
        }
    }
}

//
void pattern_ripple(float t, float ripple_time, float decay_val, int x, int y)
{
    float global_decay = 0.975;

    // 1. Apply global decay to all pixels first
    for (int i = 0; i < NUM_STRIPS; i++)
    {
        for (int j = 0; j < LEDS_PER_STRIP; j++)
        {
            shadow_leds[i][j][0] *= global_decay;
            shadow_leds[i][j][1] *= global_decay;
            shadow_leds[i][j][2] *= global_decay;

            leds[i][j].r = (uint8_t)shadow_leds[i][j][0];
            leds[i][j].g = (uint8_t)shadow_leds[i][j][1];
            leds[i][j].b = (uint8_t)shadow_leds[i][j][2];
        }
    }

    if (x < 0 || y < 0)
        return;

    // Adjust speed_multiplier to control expansion rate.
    // If t is in seconds, 5.000 means the ripple moves 5 LEDs per second.
    float speed_multiplier = 0.1;
    float current_radius = (t - ripple_time) * speed_multiplier / 10000000000000000;

    // 2. Calculate the moving ripple effect
    for (int i = 0; i < NUM_STRIPS; i++)
    {
        for (int j = 0; j < LEDS_PER_STRIP; j++)
        {
            float distance = pow(pow(i - y, 2) + pow(j - x, 2), 0.500);

            // delta is the distance between the pixel and the moving wave front
            float delta = fabs(distance - current_radius);

            // The ripple only lights up pixels near the current_radius
            float brightness = fmax(0.000, 255.000 - (decay_val * delta));

            if (brightness > 0)
            {
                uint8_t hue = (uint8_t)(t + (j * 10)) % 256;
                hue_to_rgb(hue, i, j);

                float r_val = (leds[i][j].r * brightness) / 255.000;
                float g_val = (leds[i][j].g * brightness) / 255.000;
                float b_val = (leds[i][j].b * brightness) / 255.000;

                if (r_val > shadow_leds[i][j][0])
                    shadow_leds[i][j][0] = r_val;
                if (g_val > shadow_leds[i][j][1])
                    shadow_leds[i][j][1] = g_val;
                if (b_val > shadow_leds[i][j][2])
                    shadow_leds[i][j][2] = b_val;

                leds[i][j].r = (uint8_t)shadow_leds[i][j][0];
                leds[i][j].g = (uint8_t)shadow_leds[i][j][1];
                leds[i][j].b = (uint8_t)shadow_leds[i][j][2];
            }
        }
    }
}

void pattern_column_flash(float t, int x, int y)
{
    float decay = 0.975;

    for (int i = 0; i < NUM_STRIPS; i++)
    {
        for (int j = 0; j < LEDS_PER_STRIP; j++)
        {
            shadow_leds[i][j][0] *= decay;
            shadow_leds[i][j][1] *= decay;
            shadow_leds[i][j][2] *= decay;

            leds[i][j].r = (uint8_t)shadow_leds[i][j][0];
            leds[i][j].g = (uint8_t)shadow_leds[i][j][1];
            leds[i][j].b = (uint8_t)shadow_leds[i][j][2];
        }
    }

    if (x < 0 || y < 0)
        return;

    for (int i = 0; i < NUM_STRIPS; i++)
    {
        uint8_t hue = (uint8_t)((i * 40) + (t * 100) + (y * 20)) % 256;

        hue_to_rgb(hue, i, x);

        shadow_leds[i][x][0] = (float)leds[i][x].r;
        shadow_leds[i][x][1] = (float)leds[i][x].g;
        shadow_leds[i][x][2] = (float)leds[i][x].b;
    }
}

void pattern_heat_map(int x, int y)
{
    float decay_factor = 0.975;

    for (int i = 0; i < NUM_STRIPS; i++)
    {
        for (int j = 0; j < LEDS_PER_STRIP; j++)
        {
            shadow_leds[i][j][0] *= decay_factor;
            shadow_leds[i][j][1] *= decay_factor;
            shadow_leds[i][j][2] *= decay_factor;

            leds[i][j].r = (uint8_t)shadow_leds[i][j][0];
            leds[i][j].g = (uint8_t)shadow_leds[i][j][1];
            leds[i][j].b = (uint8_t)shadow_leds[i][j][2];
        }
    }

    if (x < 0 || y < 0)
        return;

    shadow_leds[y][x][0] = 120.000; // Red
    shadow_leds[y][x][1] = 120.000; // Green
    shadow_leds[y][x][2] = 120.000; // Blue
}

void pattern_christmas(int x, int y)
{
    if (x < 0 || y < 0)
        return;

    float decay = 0.975;

    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<int> dist(0, 1279);

    for (int s = 0; s < NUM_STRIPS; s++)
    {
        for (int p = 0; p < LEDS_PER_STRIP; p++)
        {
            leds[s][p].r *= decay;
            leds[s][p].g *= decay;
            leds[s][p].b *= decay;

            int random_number = dist(gen);

            if (random_number < 256)
            {
                hue_to_rgb(random_number, s, p);
            }
        }
    }
}