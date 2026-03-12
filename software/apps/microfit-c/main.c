// MicroFit - Fitness tracker on micro:bit
//
// Step counter using ADXL335 analog accelerometer (P0, P1, P2)
// Heart rate via MAX30102 (I2C on QWIIC)
// Display on SparkFun Qwiic OLED (SSD1306 128x32)

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "app_timer.h"
#include "nrf_delay.h"
#include "nrf_twi_mngr.h"

#include "microbit_v2.h"
#include "adxl335.h"
#include "step_counter.h"
#include "max30102.h"
#include "ssd1306.h"

// I2C manager for external bus (MAX30102 + OLED on QWIIC)
NRF_TWI_MNGR_DEF(twi_mngr_external, 1, 0);

// Timers
APP_TIMER_DEF(accel_timer);   // 100Hz for step detection
APP_TIMER_DEF(pulse_timer);   // 20Hz for reading MAX30102 FIFO
APP_TIMER_DEF(display_timer); // 1Hz for display + serial output

// 100Hz: read accelerometer, feed step counter
static void accel_timer_callback(void* _unused) {
  adxl335_measurement_t accel = adxl335_read_accelerometer();
  step_counter_update(accel);
}

// 20Hz: drain MAX30102 FIFO and process samples
static void pulse_timer_callback(void* _unused) {
  max30102_update();
}

// 1Hz: update OLED and print to serial
static void display_timer_callback(void* _unused) {
  uint32_t steps = step_counter_get_steps();
  uint16_t bpm = max30102_read_bpm();
  uint32_t ir = max30102_get_ir();

  // Serial output
  printf("--- MicroFit ---\n");
  printf("Steps: %u\n", (unsigned int)steps);
  if (bpm > 0) {
    printf("Heart: %u BPM (IR: %u)\n", bpm, (unsigned int)ir);
  } else {
    printf("Heart: -- BPM (IR: %u)\n", (unsigned int)ir);
  }
  printf("\n");

  // OLED output
  char line[22]; // 128px / 6px per char = 21 chars max
  ssd1306_clear();

  ssd1306_set_cursor(0, 0);
  ssd1306_write_string("-- MicroFit --");

  ssd1306_set_cursor(0, 1);
  snprintf(line, sizeof(line), "Steps: %u", (unsigned int)steps);
  ssd1306_write_string(line);

  ssd1306_set_cursor(0, 2);
  if (bpm > 0) {
    snprintf(line, sizeof(line), "BPM: %u", bpm);
  } else {
    snprintf(line, sizeof(line), "BPM: --");
  }
  ssd1306_write_string(line);

  ssd1306_set_cursor(0, 3);
  if (!max30102_is_connected()) {
    ssd1306_write_string("No pulse sensor");
  } else if (ir < 50000) {
    ssd1306_write_string("Place finger...");
  } else {
    ssd1306_write_string("Reading...");
  }

  ssd1306_display();
}

int main(void) {
  nrf_delay_ms(500); // Wait for serial connection
  printf("\nMicroFit started!\n");

  // Initialize external I2C (QWIIC: MAX30102 + OLED)
  nrf_drv_twi_config_t i2c_config = NRF_DRV_TWI_DEFAULT_CONFIG;
  i2c_config.scl = I2C_QWIIC_SCL;
  i2c_config.sda = I2C_QWIIC_SDA;
  i2c_config.frequency = NRF_DRV_TWI_FREQ_100K;
  i2c_config.interrupt_priority = 0;
  nrf_twi_mngr_init(&twi_mngr_external, &i2c_config);

  // Initialize sensors and display
  adxl335_init();
  max30102_init(&twi_mngr_external);
  ssd1306_init(&twi_mngr_external);
  step_counter_init();

  // Set up timers
  app_timer_init();

  // Accelerometer at ~100Hz (32768 / 100 = 328 ticks)
  app_timer_create(&accel_timer, APP_TIMER_MODE_REPEATED, accel_timer_callback);
  app_timer_start(accel_timer, 328, NULL);

  // MAX30102 FIFO drain at ~20Hz (32768 / 20 = 1638 ticks)
  app_timer_create(&pulse_timer, APP_TIMER_MODE_REPEATED, pulse_timer_callback);
  app_timer_start(pulse_timer, 1638, NULL);

  // Display + serial at 1Hz
  app_timer_create(&display_timer, APP_TIMER_MODE_REPEATED, display_timer_callback);
  app_timer_start(display_timer, 32768, NULL);

  while (1) {
    nrf_delay_ms(1000);
  }
}
