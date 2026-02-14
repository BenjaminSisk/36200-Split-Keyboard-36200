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

#include "vis.h" // Custom visualization library for text graphics

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


/***********************
 * ROTATION CONFIGURATION
 * Adjust these flags to match your joystick's physical mounting.
 * This allows you to correct for any orientation without changing wiring.
 ***********************/
/**
 * @struct JoystickTransformConfig
 * @brief configuration flags for the mathematical transformation pipeline.
 * * Any of the 8 possible orientations can be represented by a unique
 * combination of these three boolean flags.
 */
typedef struct {
    bool swap_xy;   /**< If true, X and Y inputs are swapped first. */
    bool invert_x;  /**< If true, the resulting X output is inverted (Max - X). */
    bool invert_y;  /**< If true, the resulting Y output is inverted (Max - Y). */
} JoystickTransformConfig;

/**
 * @brief Global Transformation Configuration.
 * * Adjust these 3 flags to match your mounting.
 * * EXAMPLE MAPPINGS:
 * - Standard (Zero) : {false, false, false}
 * - Clockwise 90    : {true,  false, true }  (Swap -> Invert Y)
 * - Clockwise 180   : {false, true,  true }  (Invert X -> Invert Y)
 * - Clockwise 270   : {true,  true,  false}  (Swap -> Invert X)
 * Orientation,Swap XY?,Invert X?,Invert Y?
Standard (0°),false,false,false
CW 90°,true,false,true
CW 180°,false,true,true
CW 270°,true,true,false
Mirror 0°,true,false,false
Mirror 90°,false,false,true
Mirror 180°,true,true,true
Mirror 270°,false,true,false
 */
static JoystickTransformConfig g_transform_config = {
    .swap_xy  = true,   // Example: Swap X/Y
    .invert_x = false,
    .invert_y = false    // Example: Then invert Y (Result: Clockwise 90)
};


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

/**
 * @brief SERIAL PLOTTER COMPATIBLE OUTPUT
 * * USAGE: Use this if you want to see a real graph.
 * * Most IDEs (Arduino, VS Code Serial Plotter) will take this format
 * and turn it into a real-time line graph automatically.
 * * This effectively uses the HOST computer as the visualization library.
 */
void output_visualization(void) {
    // Format: "Label1:Value1, Label2:Value2"
    // The "\r\n" at the end tells the plotter "End of Frame"
    printf("X_Axis:%d,Y_Axis:%d,Btn_Signal:%d\r\n", 
           g_x_raw, 
           g_y_raw, 
           g_is_pressed ? 4095 : 0); // Scale button to max for visibility on graph
}

// -------------------------------------------------------------------------
// Module: Input Reader
// -------------------------------------------------------------------------

/**
 * @brief Reads hardware sensors and applies the 3-stage transformation pipeline.
 * * Strategy:
 * 1. Read Raw ADC.
 * 2. Stage 1: Swap Axis (Reflection).
 * 3. Stage 2: Invert X (Reflection).
 * 4. Stage 3: Invert Y (Reflection).
 * * This pipeline covers the entire Dihedral Group D4.
 */
void read_joystick_input(void) {
    // 1. Read Physical Hardware
    adc_select_input(ADC_CH_X);
    uint16_t raw_x = adc_read();

    adc_select_input(ADC_CH_Y);
    uint16_t raw_y = adc_read();

    // 2. Read Button (Logic inverted: Low = Pressed)
    g_is_pressed = !gpio_get(PIN_JOYSTICK_SW);

    // --- PIPELINE STAGE 1: SWAP XY ---
    // Mathematically equivalent to reflecting across the diagonal y=x
    if (g_transform_config.swap_xy) {
        uint16_t temp = raw_x;
        raw_x = raw_y;
        raw_y = temp;
    }

    // --- PIPELINE STAGE 2: INVERT X ---
    // Mathematically equivalent to reflecting across the Y-axis
    if (g_transform_config.invert_x) {
        raw_x = ADC_MAX_VAL - raw_x;
    }

    // --- PIPELINE STAGE 3: INVERT Y ---
    // Mathematically equivalent to reflecting across the X-axis
    if (g_transform_config.invert_y) {
        raw_y = ADC_MAX_VAL - raw_y;
    }

    // 3. Update Global State
    g_x_raw = raw_x;
    g_y_raw = raw_y;
}
// -------------------------------------------------------------------------
// Module: Output / Visualization
// -------------------------------------------------------------------------

/**
 * @brief ROBUST BACKUP: Single-line animation using Carriage Return (\r).
 * * USAGE: Use this if the multi-line box looks garbled on your terminal.
 * * LOGIC: \r moves cursor to start of line. We then overwrite the line.
 * * Extra spaces at the end ensure we clear any leftover junk characters.
 */
void output_debug_animation_single_line(void) {
    char x_bar[20];
    char y_bar[20];

    // Use the "Library" to generate visual bars
    viz_generate_bar(g_x_raw, ADC_MAX_VAL, 10, x_bar);
    viz_generate_bar(g_y_raw, ADC_MAX_VAL, 10, y_bar);

    // \r : Return to start of line (Do NOT use \n)
    printf("\rJOYSTICK: X %s %4d | Y %s %4d | BTN: %s      ", 
           x_bar, 
           g_x_raw, 
           y_bar, 
           g_y_raw, 
           g_is_pressed ? "[PRESS]" : "[     ]");
    
    // Force immediate output
    fflush(stdout); 
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
    output_debug_animation_single_line();
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
        sleep_ms(50);
    }
}