// MicroFit - Fitness tracker on micro:bit
//
// Step counter using built-in accelerometer
// Pulse sensor on EDGE_P0 (analog)

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

#include "app_timer.h"
#include "nrf_delay.h"
#include "nrf_twi_mngr.h"

#include "microbit_v2.h"
#include "lsm303agr.h"
#include "step_counter.h"
#include "pulse_sensor.h"

// I2C manager for internal bus (accelerometer)
NRF_TWI_MNGR_DEF(twi_mngr_internal, 1, 0);

// Timers
APP_TIMER_DEF(accel_timer);   // 100Hz for step detection
APP_TIMER_DEF(pulse_timer);   // 500Hz for pulse sensor sampling
APP_TIMER_DEF(display_timer); // 1Hz for serial output

// 100Hz: read accelerometer, feed step counter
static void accel_timer_callback(void* _unused) {
  lsm303agr_measurement_t accel = lsm303agr_read_accelerometer();
  step_counter_update(accel);
}

// 500Hz: sample pulse sensor
static void pulse_timer_callback(void* _unused) {
  pulse_sensor_update();
}

// 1Hz: print status
static void display_timer_callback(void* _unused) {
  uint32_t steps = step_counter_get_steps();
  float temp = lsm303agr_read_temperature();
  uint16_t bpm = pulse_sensor_read_bpm();
  uint16_t signal = pulse_sensor_get_signal();

  printf("--- MicroFit ---\n");
  printf("Steps: %lu\n", steps);
  printf("Temp:  %.1f C\n", temp);
  if (bpm > 0) {
    printf("Heart: %u BPM (signal: %u)\n", bpm, signal);
  } else {
    printf("Heart: -- BPM (signal: %u)\n", signal);
  }
  printf("\n");
}

int main(void) {
  printf("MicroFit started!\n");

  // Initialize internal I2C (accelerometer)
  nrf_drv_twi_config_t i2c_config = NRF_DRV_TWI_DEFAULT_CONFIG;
  i2c_config.scl = I2C_INTERNAL_SCL;
  i2c_config.sda = I2C_INTERNAL_SDA;
  i2c_config.frequency = NRF_DRV_TWI_FREQ_100K;
  i2c_config.interrupt_priority = 0;
  nrf_twi_mngr_init(&twi_mngr_internal, &i2c_config);

  // Initialize sensors
  lsm303agr_init(&twi_mngr_internal);
  pulse_sensor_init();
  step_counter_init();

  // Set up timers
  app_timer_init();

  // Accelerometer at ~100Hz (32768 / 100 = 328 ticks)
  app_timer_create(&accel_timer, APP_TIMER_MODE_REPEATED, accel_timer_callback);
  app_timer_start(accel_timer, 328, NULL);

  // Pulse sensor at ~500Hz (32768 / 500 ≈ 66 ticks)
  app_timer_create(&pulse_timer, APP_TIMER_MODE_REPEATED, pulse_timer_callback);
  app_timer_start(pulse_timer, 66, NULL);

  // Display at 1Hz
  app_timer_create(&display_timer, APP_TIMER_MODE_REPEATED, display_timer_callback);
  app_timer_start(display_timer, 32768, NULL);

  while (1) {
    nrf_delay_ms(1000);
  }
}
