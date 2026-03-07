// =========================================================================
// MODULE: MINI_VIZ LIBRARY (Embedded Visualization Helpers)
// =========================================================================
// Think of this as your "Imported Library" for text graphics.

typedef struct {
    char buffer[128];
    int width;
} ProgressBar;

/** * @brief Generates a text-based progress bar: "[#####-----]"
 * @param val Current value
 * @param max Maximum value
 * @param width Width of the bar in characters
 * @param output_buf Buffer to store the resulting string
 */
void viz_generate_bar(int val, int max, int width, char* output_buf) {
    int filled_len = (val * width) / max;
    if (filled_len > width) filled_len = width;
    if (filled_len < 0) filled_len = 0;

    output_buf[0] = '[';
    for (int i = 0; i < width; i++) {
        if (i < filled_len) output_buf[i+1] = '#'; // Filled char
        else output_buf[i+1] = '-';                // Empty char
    }
    output_buf[width+1] = ']';
    output_buf[width+2] = '\0'; // Null terminator
}