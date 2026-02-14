#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

// Define the GPIO pins we are using
const uint JOYSTICK_X_PIN = 26; // ADC0
const uint JOYSTICK_Y_PIN = 27; // ADC1
const uint JOYSTICK_SW_PIN = 22; // Digital Button

int main() {
    stdio_init_all();
    printf("RP2350 Joystick Test Starting...\n");

    // --- Initialize ADC (Analog to Digital Converter) ---
    adc_init();

    // Make sure the GPIO pins are set to high-impedance (ADC mode)
    adc_gpio_init(JOYSTICK_X_PIN);
    adc_gpio_init(JOYSTICK_Y_PIN);

    // --- Initialize Button (Digital Input) ---
    gpio_init(JOYSTICK_SW_PIN);
    gpio_set_dir(JOYSTICK_SW_PIN, GPIO_IN);
    // Enable the internal pull-up resistor. 
    // The switch connects to GND when pressed, so logic is inverted (Low = Pressed).
    gpio_pull_up(JOYSTICK_SW_PIN);

    while (true) {
        // Read X-Axis (Connected to ADC0 / GP26)
        adc_select_input(0); 
        uint16_t x_raw = adc_read();

        // Read Y-Axis (Connected to ADC1 / GP27)
        adc_select_input(1); 
        uint16_t y_raw = adc_read();

        // Read Switch (0 = Pressed, 1 = Not Pressed)
        bool sw_state = gpio_get(JOYSTICK_SW_PIN);

        // --- Output Data ---
        // ADC values range from 0 to 4095 (12-bit)
        // Center is approx 2048
        printf("X: %d | Y: %d | Button: %s\n", 
               x_raw, 
               y_raw, 
               sw_state ? "Released" : "PRESSED");

        sleep_ms(100); // Delay to make the serial output readable
    }
}