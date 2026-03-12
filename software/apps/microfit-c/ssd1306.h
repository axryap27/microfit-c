// SSD1306 OLED display driver (128x32, I2C)

#pragma once

#include <stdint.h>
#include "nrf_twi_mngr.h"

// I2C address
static const uint8_t SSD1306_ADDRESS = 0x3C;

// Display dimensions
#define SSD1306_WIDTH  128
#define SSD1306_HEIGHT 32
#define SSD1306_PAGES  4  // 32 pixels / 8 pixels per page

// Initialize the SSD1306 display
void ssd1306_init(const nrf_twi_mngr_t* i2c);

// Clear the entire display
void ssd1306_clear(void);

// Set cursor position for text
// col: 0-127, page: 0-3
void ssd1306_set_cursor(uint8_t col, uint8_t page);

// Write a string at the current cursor position
void ssd1306_write_string(const char* str);

// Update the display (flush buffer)
void ssd1306_display(void);
