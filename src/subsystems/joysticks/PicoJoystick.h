#pragma once

#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/timer.h"

struct JoystickTransformConfig {
    bool swap_xy;   
    bool invert_x;  
    bool invert_y;  
};

class PicoJoystick {
private:
    // Hardware mappings
    uint8_t pin_x;
    uint8_t adc_ch_x;
    uint8_t pin_y;
    uint8_t adc_ch_y;
    uint8_t pin_sw;

    // Configuration & Tuning
    JoystickTransformConfig config;
    int32_t timer_interval_ms;
    float ema_alpha; // Smoothing factor: 0.0 (max smoothing) to 1.0 (no smoothing)

    // Internal State
    uint16_t x_raw;
    uint16_t y_raw;
    uint16_t x_filtered;
    uint16_t y_filtered;
    bool is_pressed;

    struct repeating_timer timer;
    static const uint16_t ADC_MAX_VAL = 4095;

    // Internal modular functions
    void applyEMA(uint16_t current_x, uint16_t current_y);
    static bool timerCallback(struct repeating_timer *t);

public:
    // Overloaded Default Constructor (Delegates to Parameterized)
    PicoJoystick();

    // Parameterized Constructor
    PicoJoystick(uint8_t pin_x, uint8_t adc_ch_x, 
                 uint8_t pin_y, uint8_t adc_ch_y, 
                 uint8_t pin_sw, 
                 JoystickTransformConfig config,
                 int32_t timer_interval_ms,
                 float ema_alpha);

    void init();
    void startTimer();

    /**
     * @brief Rapid initialization for testing environments.
     * Calls init() and startTimer() sequentially.
     */
    void testing_QuickInit();

    void update(); // Now meant to be called by the IRQ
    
    void debugPrintSingleLine() const;

    /**
     * @brief Unified debug print interface.
     * Currently wraps debugPrintSingleLine().
     */
    void debugPrint() const;

    // Getters
    uint16_t getX() const { return x_filtered; }
    uint16_t getY() const { return y_filtered; }
    bool getPressed() const { return is_pressed; }
};