#pragma once

#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/timer.h"
#include "hardwareMap.h"

#include <functional>

struct JoystickTransformConfig
{
    bool swap_xy;
    bool invert_x;
    bool invert_y;
};

class PicoJoystick
{

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

    /**
     * @brief initialize hardware and optionally start the timer immediately.
     * @param trigger If true, starts the timer immediately after initialization.
     */
    void init(bool trigger = false);

    /*
    * @brief Start the repeating timer to update joystick readings at regular intervals.
     * This is typically called after initialization to begin the periodic polling of the joystick.
    */ 
    void startTimer();


    void update(); // Now meant to be called by the IRQ


    // Enums are great here because they provide strict type safety at compile time
    enum class Axis { X, Y };
    // Callbacks
    void setOnChange(std::function<void(Axis, uint8_t)> callback);

//===================================================================================================================================
    //direct modifiers

    // Getters

    /// @brief get the current filtered X value (0-4095)
    /// @return
    uint16_t getX() const;

    /// @brief get the current filtered Y value (0-4095)
    /// @return 
    uint16_t getY() const;

    /// @brief Check if the joystick button is currently pressed.
    /// @return true if pressed, false otherwise.
    bool getPressed() const;

    // Setters
    
    // @brief Directly set the raw X and Y values, bypassing ADC reads. Useful for emulation.
    void setPosition(uint16_t x, uint16_t y);

//===================================================================================================================================
//debug:
    /**
     * @brief Manually injects X and Y axis values for emulation, bypassing the ADC.
     * @param x Simulated 16-bit X-axis value (0-4095).
     * @param y Simulated 16-bit Y-axis value (0-4095).
     */
    void simulatePosition(uint16_t x, uint16_t y);


    /**
     * @brief Writes a visual representation of the joystick state into a provided buffer.
     * @param buffer Pointer to a character array.
     * @param maxLength The maximum number of bytes that can safely be written.
     */
    void toString(char* buffer, size_t maxLength) const;

//===================================================================================================================================

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
    uint16_t x_current;
    uint16_t y_current;
    bool is_pressed;

    struct repeating_timer timer;
    static const uint16_t ADC_MAX_VAL = 4095;

    /**
     * @brief Sets the callback fired 
     * @param callback Function 
     */
    std::function<void(Axis, uint8_t)> onChangeCb;


    // 8-bit State Tracking
    uint8_t last_x_8bit = 0;
    uint8_t last_y_8bit = 0;

    // Internal modular functions
    void applyEMA(uint16_t x_initial, uint16_t y_initial);
    static bool timerCallback(struct repeating_timer *t);

};

