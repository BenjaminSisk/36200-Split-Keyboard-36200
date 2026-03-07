#include "Joysticks/PicoJoystick.h"
#include <stdio.h>

// Constructor Delegation: Default calls Parameterized
PicoJoystick::PicoJoystick() 
    /*testing: 
     * - pin_x = 40, adc_ch_x = 0
     * - pin_y = 41, adc_ch_y = 1
     * - pin_sw = 42
     * - swap_xy = true, invert_x = true, invert_y = false
     * - timer_interval_ms = -10 (negative for exact delay between executions)
     * - ema_alpha = 0.2 (moderate smoothing)
     */
    : PicoJoystick(40, 0, 41, 1, 42, {true, true, false}, -10, 0.2f) {}

// Parameterized Constructor
PicoJoystick::PicoJoystick(uint8_t pin_x, uint8_t adc_ch_x, 
                           uint8_t pin_y, uint8_t adc_ch_y, 
                           uint8_t pin_sw, 
                           JoystickTransformConfig config,
                           int32_t timer_interval_ms,
                           float ema_alpha)
    : pin_x(pin_x), adc_ch_x(adc_ch_x), 
      pin_y(pin_y), adc_ch_y(adc_ch_y), 
      pin_sw(pin_sw), config(config), 
      timer_interval_ms(timer_interval_ms), ema_alpha(ema_alpha),
      x_raw(2048), y_raw(2048), x_filtered(2048), y_filtered(2048), is_pressed(false) {}

void PicoJoystick::init() {
    adc_gpio_init(pin_x);
    adc_gpio_init(pin_y);
    gpio_init(pin_sw);
    gpio_set_dir(pin_sw, GPIO_IN);
    gpio_pull_up(pin_sw);
}

// Static wrapper to bridge C++ object instance with C-style hardware timer callback
bool PicoJoystick::timerCallback(struct repeating_timer *t) {
    PicoJoystick* instance = static_cast<PicoJoystick*>(t->user_data);
    if (instance) {
        instance->update();
    }
    return true; // Return true to keep timer repeating
}

void PicoJoystick::startTimer() {
    // Negative interval means exact delay between executions (deterministic)
    add_repeating_timer_ms(timer_interval_ms, timerCallback, this, &timer);
}

void PicoJoystick::applyEMA(uint16_t current_x, uint16_t current_y) {
    // Exponential Moving Average implementation
    x_filtered = (uint16_t)((ema_alpha * current_x) + ((1.0f - ema_alpha) * x_filtered));
    y_filtered = (uint16_t)((ema_alpha * current_y) + ((1.0f - ema_alpha) * y_filtered));
}

void PicoJoystick::update() {
    adc_select_input(adc_ch_x);
    uint16_t current_x = adc_read();

    adc_select_input(adc_ch_y);
    uint16_t current_y = adc_read();

    is_pressed = !gpio_get(pin_sw);

    // Pipeline Transform
    if (config.swap_xy) {
        uint16_t temp = current_x;
        current_x = current_y;
        current_y = temp;
    }
    if (config.invert_x) {
        current_x = ADC_MAX_VAL - current_x;
    }
    if (config.invert_y) {
        current_y = ADC_MAX_VAL - current_y;
    }

    x_raw = current_x;
    y_raw = current_y;

    // Apply Noise Filter
    applyEMA(current_x, current_y);
}

// void PicoJoystick::debugPrintSingleLine() const {
//     printf("\rJOYSTICK [EMA]: X %4d | Y %4d | BTN: %s      ", 
//            x_filtered, y_filtered, is_pressed ? "[PRESS]" : "[     ]");
//     fflush(stdout); 
// }
void PicoJoystick::debugPrintSingleLine() const {
    const int bar_width = 10;
    
    // Arrays to hold: '[' + 10 chars + ']' + null terminator
    char x_bar[bar_width + 3]; 
    char y_bar[bar_width + 3]; 

    // Calculate filled positions based on the filtered EMA values
    int x_hashes = (x_filtered * bar_width) / ADC_MAX_VAL;
    int y_hashes = (y_filtered * bar_width) / ADC_MAX_VAL;

    // Clamp values to strictly prevent buffer overflow in edge cases
    if (x_hashes > bar_width) x_hashes = bar_width;
    if (y_hashes > bar_width) y_hashes = bar_width;

    // Build X Bar string: e.g., "[###       ]"
    x_bar[0] = '[';
    for (int i = 0; i < bar_width; i++) {
        x_bar[i + 1] = (i < x_hashes) ? '#' : ' ';
    }
    x_bar[bar_width + 1] = ']';
    x_bar[bar_width + 2] = '\0';

    // Build Y Bar string: e.g., "[###       ]"
    y_bar[0] = '[';
    for (int i = 0; i < bar_width; i++) {
        y_bar[i + 1] = (i < y_hashes) ? '#' : ' ';
    }
    y_bar[bar_width + 1] = ']';
    y_bar[bar_width + 2] = '\0';

    // Print the formatted string, overwriting the current line (\r)
    printf("\rJOYSTICK [EMA]: X %s %4d | Y %s %4d | BTN: %s      ", 
           x_bar, x_filtered, 
           y_bar, y_filtered, 
           is_pressed ? "[PRESS]" : "[     ]");
    
    // Force immediate output to the console
    fflush(stdout); 
}

void PicoJoystick::testing_QuickInit() {
    init();
    startTimer();
}

void PicoJoystick::debugPrint() const {
    debugPrintSingleLine();
}