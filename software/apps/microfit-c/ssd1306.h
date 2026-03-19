// SH1107 OLED display driver (128x64, I2C)
// SparkFun Qwiic OLED 1.3"

#pragma once

#include <stdbool.h>
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

// Set or clear a single pixel
void ssd1306_draw_pixel(uint8_t x, uint8_t y, bool on);

// Draw a horizontal line
void ssd1306_draw_hline(uint8_t x, uint8_t y, uint8_t width);

// Draw a 1-bit bitmap (column-major, same format as font)
// w must be <= 128, h must be a multiple of 8
void ssd1306_draw_bitmap(uint8_t x, uint8_t page, const uint8_t* bitmap, uint8_t w, uint8_t pages);

// Write a string scaled 2x (each char becomes 12x16 pixels, spans 2 pages)
void ssd1306_write_string_2x(const char* str);

// Update the display (flush buffer)
void ssd1306_display(void);
