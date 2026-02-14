#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

// --- Configuration for RP2350 QFN-80 ---
// ADC Channel 0 is on GPIO 40
const uint JOYSTICK_X_PIN = 40; 
const uint JOYSTICK_X_ADC_CHANNEL = 0;

// ADC Channel 1 is on GPIO 41
const uint JOYSTICK_Y_PIN = 41; 
const uint JOYSTICK_Y_ADC_CHANNEL = 1;

// Digital Button (Moved to GPIO 38)
const uint JOYSTICK_SW_PIN = 38; 

int main() {
    stdio_init_all();
    printf("RP2350 QFN-80 Joystick Test Starting...\n");

    // --- 1. Initialize ADC ---
    adc_init();

    // Prepare GPIO 40 and 41 for Analog function
    // This disables digital logic on these pins to allow accurate analog reads
    adc_gpio_init(JOYSTICK_X_PIN);
    adc_gpio_init(JOYSTICK_Y_PIN);

    // --- 2. Initialize Button (Digital Input) ---
    gpio_init(JOYSTICK_SW_PIN);
    gpio_set_dir(JOYSTICK_SW_PIN, GPIO_IN);
    // Enable internal pull-up. Button connects to GND when pressed.
    gpio_pull_up(JOYSTICK_SW_PIN);

    while (true) {
        // --- Read X-Axis ---
        adc_select_input(JOYSTICK_X_ADC_CHANNEL); 
        uint16_t x_raw = adc_read();

        // --- Read Y-Axis ---
        adc_select_input(JOYSTICK_Y_ADC_CHANNEL); 
        uint16_t y_raw = adc_read();

        // --- Read Switch ---
        // logic: 0 = Pressed, 1 = Released
        bool sw_pressed = !gpio_get(JOYSTICK_SW_PIN); 

        // --- Output Data ---
        // 12-bit ADC resolution (0 - 4095)
        printf("X: %4d | Y: %4d | Button: %s\n", 
               x_raw, 
               y_raw, 
               sw_pressed ? "PRESSED" : "Open");

        sleep_ms(100); 
    }
}