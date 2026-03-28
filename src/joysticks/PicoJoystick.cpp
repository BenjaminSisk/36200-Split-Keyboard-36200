#include "PicoJoystick.h"
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
: PicoJoystick(hardwareMap::Pins::JOYSTICK_X, 0, // ADC CH 0
                   hardwareMap::Pins::JOYSTICK_Y, 1, // ADC CH 1
                   hardwareMap::Pins::JOYSTICK_SW, 
                   {true, true, false}, -10, 0.2f) {}

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
      x_raw(2048), y_raw(2048), x_current(2048), y_current(2048), is_pressed(false) {}

void PicoJoystick::init(bool trigger) {
    //initialize hardware pins
    adc_gpio_init(pin_x);
    adc_gpio_init(pin_y);
    gpio_init(pin_sw);

    //configs
    gpio_set_dir(pin_sw, GPIO_IN);
    gpio_pull_up(pin_sw);

    //enables

    //outputs
    if (trigger) {
        startTimer(); // Start the timer immediately. 
    }


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





void PicoJoystick::update() {
    // GUARD: If this is a virtual joystick (NO_PIN), skip hardware reads.
    // This allows the TerminalEmulator's simulated values to persist!
    if (pin_x == hardwareMap::NO_PIN || pin_y == hardwareMap::NO_PIN) {
        return; 
    }

    adc_select_input(adc_ch_x);
    uint16_t x_initial = adc_read();

    adc_select_input(adc_ch_y);
    uint16_t y_initial = adc_read();

    is_pressed = !gpio_get(pin_sw);

    // Pipeline Transform
    if (config.swap_xy) {
        uint16_t temp = x_initial;
        x_initial = y_initial;
        y_initial = temp;
    }
    if (config.invert_x) {
        x_initial = ADC_MAX_VAL - x_initial;
    }
    if (config.invert_y) {
        y_initial = ADC_MAX_VAL - y_initial;
    }

    x_raw = x_initial;
    y_raw = y_initial;

    // Apply Noise Filter
    applyEMA(x_initial, y_initial);

    uint8_t current_x_8bit = static_cast<uint8_t>(x_current >> 4);
    uint8_t current_y_8bit = static_cast<uint8_t>(y_current >> 4);

    if (current_x_8bit != last_x_8bit) {
        last_x_8bit = current_x_8bit;
        if (onChangeCb) onChangeCb(Axis::X, current_x_8bit); // Fire with X enum
    }

    if (current_y_8bit != last_y_8bit) {
        last_y_8bit = current_y_8bit;
        if (onChangeCb) onChangeCb(Axis::Y, current_y_8bit); // Fire with Y enum
    }
}



void PicoJoystick::setOnChange(std::function<void(Axis, uint8_t)> callback) {
    onChangeCb = callback;
}



//===================================================================================================================================
//helpers:
//===================================================================================================================================

void PicoJoystick::applyEMA(uint16_t x_initial, uint16_t y_initial) {
    // Exponential Moving Average implementation
    x_current = (uint16_t)((ema_alpha * x_initial) + ((1.0f - ema_alpha) * x_current));
    y_current = (uint16_t)((ema_alpha * y_initial) + ((1.0f - ema_alpha) * y_current));
}

//getters / setters:


uint16_t PicoJoystick::getX() const {
    return x_current; 
}

uint16_t PicoJoystick::getY() const {
    return y_current; 
}

bool PicoJoystick::getPressed() const {
    return is_pressed;
}

// Setters
    
void PicoJoystick::setPosition(uint16_t x, uint16_t y) {
    x_raw = x;
    y_raw = y;
    x_current = x;
    y_current = y;

}

//===================================================================================================================================
//debug:
//===================================================================================================================================

void PicoJoystick::simulatePosition(uint16_t x, uint16_t y) {
    printf("[DEBUG] Simulating Joystick Position. X: %d, Y: %d\n", x, y);
    setPosition(x, y); // Scale 0-255 to 0-4095
}


void PicoJoystick::toString(char* buffer, size_t maxLength) const {
    const int bar_width = 10;
    char x_bar[bar_width + 3]; 
    char y_bar[bar_width + 3]; 

    int x_hashes = (x_current * bar_width) / ADC_MAX_VAL;
    int y_hashes = (y_current * bar_width) / ADC_MAX_VAL;

    if (x_hashes > bar_width) x_hashes = bar_width;
    if (y_hashes > bar_width) y_hashes = bar_width;

    x_bar[0] = '[';
    for (int i = 0; i < bar_width; i++) x_bar[i + 1] = (i < x_hashes) ? '#' : ' ';
    x_bar[bar_width + 1] = ']'; x_bar[bar_width + 2] = '\0';

    y_bar[0] = '[';
    for (int i = 0; i < bar_width; i++) y_bar[i + 1] = (i < y_hashes) ? '#' : ' ';
    y_bar[bar_width + 1] = ']'; y_bar[bar_width + 2] = '\0';

    // snprintf is the SAFE way to format strings in C/C++. 
    // It guarantees it will never write past maxLength, preventing buffer overflow exploits/crashes.
    snprintf(buffer, maxLength, "X %s %4d | Y %s %4d | BTN: %s", x_bar, x_current, y_bar, y_current, is_pressed ? "[PRESS]" : "[     ]");

}