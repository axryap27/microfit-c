#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// Initialize the LED matrix display
void led_matrix_init(void);

// Set the LED matrix display buffer (5x5 grid of booleans)
void led_matrix_set(bool grid[5][5]);

// Refresh the LED matrix display (strobe all rows once, ~5ms)
// Call this rapidly in a loop to maintain a visible display
void led_matrix_refresh(void);

