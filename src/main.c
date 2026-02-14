/**
 * @file joystick_main.c
 * @brief Modular Joystick Interface for RP2350 (QFN-80 Package).
 * * This program initializes the ADC and GPIOs for a standard analog joystick,
 * updates file-global state variables, and renders a text-based animation
 * to the serial console.
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

// -------------------------------------------------------------------------
// Constants & Configuration
// -------------------------------------------------------------------------

/** @brief GPIO pin for Joystick X-Axis (QFN-80: ADC Ch 0) */
#define PIN_JOYSTICK_X      40
/** @brief ADC Channel for X-Axis */
#define ADC_CH_X            0

/** @brief GPIO pin for Joystick Y-Axis (QFN-80: ADC Ch 1) */
#define PIN_JOYSTICK_Y      41
/** @brief ADC Channel for Y-Axis */
#define ADC_CH_Y            1

/** @brief GPIO pin for Joystick Switch (Digital Input) */
#define PIN_JOYSTICK_SW     38

/** @brief The raw ADC resolution (12-bit = 0-4095) */
#define ADC_MAX_VAL         4095

// -------------------------------------------------------------------------
// File-Global State Variables (Static for Compartmentalization)
// -------------------------------------------------------------------------

/** * @brief Raw ADC value for X-axis (0-4095).
 * @note static keyword limits scope to this file. 
 */
static uint16_t g_x_raw = 2048;

/** * @brief Raw ADC value for Y-axis (0-4095).
 * @note static keyword limits scope to this file. 
 */
static uint16_t g_y_raw = 2048;

/** * @brief Current button state (true = pressed, false = released).
 * @note static keyword limits scope to this file. 
 */
static bool g_is_pressed = false;

// -------------------------------------------------------------------------
// Module: Initialization
// -------------------------------------------------------------------------

/**
 * @brief Initializes all necessary hardware peripherals.
 * * Sets up stdio for serial output, initializes the ADC hardware,
 * configures the specific GPIO pins for analog input, and sets up
 * the digital switch with an internal pull-up resistor.
 */
void system_init(void) {
    // 1. Initialize Serial Communication (USB)
    stdio_init_all();
    
    // 2. Initialize ADC Hardware
    adc_init();

    // 3. Configure Analog Pins (Disable digital buffers)
    // Note: On RP2350 QFN-80, these are GPIO 40 and 41.
    adc_gpio_init(PIN_JOYSTICK_X);
    adc_gpio_init(PIN_JOYSTICK_Y);

    // 4. Configure Digital Switch Pin
    gpio_init(PIN_JOYSTICK_SW);
    gpio_set_dir(PIN_JOYSTICK_SW, GPIO_IN);
    // Enable pull-up: Pin reads HIGH (1) when open, LOW (0) when pressed.
    gpio_pull_up(PIN_JOYSTICK_SW);

    // Clear terminal screen using ANSI escape code
    printf("\033[2J"); 
    printf("System Initialized. Starting Main Loop...\n");
}

// -------------------------------------------------------------------------
// Module: Input Reader
// -------------------------------------------------------------------------

/**
 * @brief Reads the hardware sensors and updates file-global variables.
 * * Performs three distinct reads:
 * 1. Switches ADC mux to X channel and samples.
 * 2. Switches ADC mux to Y channel and samples.
 * 3. Reads the digital logic level of the switch pin.
 */
void read_joystick_input(void) {
    // Read X Axis
    adc_select_input(ADC_CH_X);
    g_x_raw = adc_read();

    // Read Y Axis
    adc_select_input(ADC_CH_Y);
    g_y_raw = adc_read();

    // Read Button (Invert logic because of pull-up: Low = Pressed)
    g_is_pressed = !gpio_get(PIN_JOYSTICK_SW);
}

// -------------------------------------------------------------------------
// Module: Output / Visualization
// -------------------------------------------------------------------------
/**
 * @brief Renders the frame using efficient terminal manipulation.
 * * Uses putchar() for the grid to minimize overhead and fflush() 
 * to ensure the frame renders instantly as one block.
 */
void output_debug_animation(void) {
    const int GRID_W = 15;
    const int GRID_H = 9;
    
    // 1. Calculate Cursor Position relative to grid
    int pos_x = (g_x_raw * GRID_W) / ADC_MAX_VAL;
    int pos_y = (g_y_raw * GRID_H) / ADC_MAX_VAL;

    // Clamp values
    if (pos_x >= GRID_W) pos_x = GRID_W - 1;
    if (pos_y >= GRID_H) pos_y = GRID_H - 1;

    // 2. Prepare Frame Start
    // \033[H : Move cursor to Top-Left (Home). Do NOT clear screen (prevents flicker).
    // printf("\033[H"); 

    // 3. Print Header Information
    // printf("=== RP2350 VISUALIZER ===\n");
    printf("X: %04d | Y: %04d | BTN: %s  \n\r", // Extra spaces at end to overwrite old text
           g_x_raw, 
           g_y_raw, 
           g_is_pressed ? "ON " : "OFF");
    // printf("-------------------------\n");

    // // 4. Render Grid using putchar
    // for (int y = 0; y < GRID_H; y++) {
    //     putchar('|'); // Left Border

    //     for (int x = 0; x < GRID_W; x++) {
    //         if (x == pos_x && y == pos_y) {
    //             // Joystick Cursor
    //             if (g_is_pressed) {
    //                 putchar('X'); 
    //             } else {
    //                 putchar('O');
    //             }
    //         } else if (x == GRID_W/2 && y == GRID_H/2) {
    //             // Center Crosshair
    //             putchar('+'); 
    //         } else {
    //             // Empty Space
    //             putchar(' '); 
    //         }
    //     }
        
    //     putchar('|'); // Right Border
    //     putchar('\n'); // End of row
    // }
    // printf("-------------------------\n");

    // // 5. Clean up remainder of screen
    // // \033[J : Clear from cursor to end of screen. 
    // // This removes any artifacts if the previous frame was somehow longer.
    // printf("\033[J");

    // // 6. Force Output
    // // Important: stdout is buffered. fflush forces the buffer to be written 
    // // to the serial port NOW, ensuring the whole frame appears at once.
    // fflush(stdout); 
}

// -------------------------------------------------------------------------
// Main Callback & Loop
// -------------------------------------------------------------------------

/**
 * @brief The primary application logic callback.
 * * This function is designed to be called repeatedly by the main loop.
 * It orchestrates the flow of data from input -> file-global state -> output.
 */
void main_loop_callback(void) {
    // 1. Acquire Data (updates static variables)
    read_joystick_input();

    // 2. Visualize Data (reads static variables)
    output_debug_animation();
}

/**
 * @brief Main Entry Point.
 * * Initializes the system and enters a permanent sleep-loop that triggers
 * the application callback at a fixed rate.
 * * @return int Standard exit code (never reached in embedded).
 */
int main() {
    // Initialization Phase
    system_init();

    // Loop Phase
    while (true) {
        main_loop_callback();
        
        // 50ms delay = ~20 frames per second
        sleep_ms(500);
    }
}