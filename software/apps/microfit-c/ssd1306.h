// SH1107 OLED display driver (128x64, I2C)
// SparkFun Qwiic OLED 1.3"

#pragma once

#include <stdint.h>
#include "nrf_twi_mngr.h"

// I2C address
static const uint8_t SH1107_ADDRESS = 0x3D;

// Display dimensions
#define OLED_WIDTH  128
#define OLED_HEIGHT 64
#define OLED_PAGES  8  // 64 pixels / 8 pixels per page

// Initialize the display
void ssd1306_init(const nrf_twi_mngr_t* i2c);

// Clear the entire display
void ssd1306_clear(void);

// Set cursor position for text
// col: 0-127, page: 0-7
void ssd1306_set_cursor(uint8_t col, uint8_t page);

// Write a string at the current cursor position
void ssd1306_write_string(const char* str);

// Update the display (flush buffer)
void ssd1306_display(void);
