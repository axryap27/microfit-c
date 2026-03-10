// LED Matrix Driver
// Displays characters on the LED matrix

#include <stdbool.h>
#include <stdio.h>

#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "app_timer.h"

#include "led_matrix.h"
#include "font.h"
#include "microbit_v2.h"


void led_matrix_init(void) {
  // Arrays of row and column pin numbers for easier iteration
  uint32_t row_leds[] = {LED_ROW1, LED_ROW2, LED_ROW3, LED_ROW4, LED_ROW5};
  uint32_t col_leds[] = {LED_COL1, LED_COL2, LED_COL3, LED_COL4, LED_COL5};

  // initialize row pins as outputs and clear them (LOW = inactive)
  for (int i = 0; i < 5; i++) {
    nrf_gpio_pin_dir_set(row_leds[i], NRF_GPIO_PIN_DIR_OUTPUT);
    nrf_gpio_pin_clear(row_leds[i]);
  }

  // initialize col pins as outputs and clear them (LOW = LED would turn on if row is HIGH)
  for (int i = 0; i < 5; i++) {
    nrf_gpio_pin_dir_set(col_leds[i], NRF_GPIO_PIN_DIR_OUTPUT);
    nrf_gpio_pin_clear(col_leds[i]);
  }

  // initialize timer(s) (Step 2 and onwards)

  // set default state for the LED display (Step 3 and onwards)

  // Test: Enable 4 LEDs in row 1 (columns 1-4 on, column 5 off)
  nrf_gpio_pin_set(LED_ROW1);     // Activate row 1 (HIGH)
  nrf_gpio_pin_clear(LED_COL1);   // Column 1 LOW = LED ON
  nrf_gpio_pin_clear(LED_COL2);   // Column 2 LOW = LED ON
  nrf_gpio_pin_clear(LED_COL3);   // Column 3 LOW = LED ON
  nrf_gpio_pin_clear(LED_COL4);   // Column 4 LOW = LED ON
  nrf_gpio_pin_set(LED_COL5);     // Column 5 HIGH = LED OFF
}

static bool display_buffer[5][5] = {{false}};

void led_matrix_set(bool grid[5][5]) {
  memcpy(display_buffer, grid, sizeof(display_buffer));
}

void led_matrix_refresh(void) {
  uint32_t row_pins[] = {LED_ROW1, LED_ROW2, LED_ROW3, LED_ROW4, LED_ROW5};
  uint32_t col_pins[] = {LED_COL1, LED_COL2, LED_COL3, LED_COL4, LED_COL5};

  for (int row = 0; row < 5; row++) {
    // disable all rows
    for (int r = 0; r < 5; r++) {
      nrf_gpio_pin_clear(row_pins[r]);
    }

    // set columns for this row (LOW = LED on, HIGH = LED off)
    for (int col = 0; col < 5; col++) {
      if (display_buffer[row][col]) {
        nrf_gpio_pin_clear(col_pins[col]);
      } else {
        nrf_gpio_pin_set(col_pins[col]);
      }
    }

    // enable this row
    nrf_gpio_pin_set(row_pins[row]);
    nrf_delay_us(1000);
  }

  // disable all rows after strobing
  for (int r = 0; r < 5; r++) {
    nrf_gpio_pin_clear(row_pins[r]);
  }
}
