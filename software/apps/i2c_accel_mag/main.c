// I2C accelerometer/magnetometer app
//
// Read from I2C accelerometer/magnetometer on the Microbit

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

#include "app_timer.h"
#include "nrf_delay.h"
#include "nrf_twi_mngr.h"

#include "microbit_v2.h"
#include "lsm303agr.h"

// Global variables
NRF_TWI_MNGR_DEF(twi_mngr_instance, 1, 0);

// App timer for periodic temperature reading
APP_TIMER_DEF(temp_timer);

static void sensor_timer_callback(void* _unused) {
  float temp = lsm303agr_read_temperature();
  lsm303agr_measurement_t accel = lsm303agr_read_accelerometer();
  lsm303agr_measurement_t mag = lsm303agr_read_magnetometer();

  printf("Temp: %f C\n", temp);
  printf("Accel (g): X: %f  Y: %f  Z: %f\n", accel.x_axis, accel.y_axis, accel.z_axis);
  printf("Mag (uT):  X: %f  Y: %f  Z: %f\n", mag.x_axis, mag.y_axis, mag.z_axis);
  float tilt = lsm303agr_tilt_angle(accel);
  printf("Tilt (deg): %f\n\n", tilt);
}

int main(void) {
  printf("Board started!\n");

  // Initialize I2C peripheral and driver
  nrf_drv_twi_config_t i2c_config = NRF_DRV_TWI_DEFAULT_CONFIG;
  // WARNING!!
  // These are NOT the correct pins for external I2C communication.
  // If you are using QWIIC or other external I2C devices, the are
  // connected to EDGE_P19 (a.k.a. I2C_QWIIC_SCL) and EDGE_P20 (a.k.a. I2C_QWIIC_SDA)
  i2c_config.scl = I2C_INTERNAL_SCL;
  i2c_config.sda = I2C_INTERNAL_SDA;
  i2c_config.frequency = NRF_DRV_TWI_FREQ_100K;
  i2c_config.interrupt_priority = 0;
  nrf_twi_mngr_init(&twi_mngr_instance, &i2c_config);

  // Initialize the LSM303AGR accelerometer/magnetometer sensor
  lsm303agr_init(&twi_mngr_instance);

  // Initialize and start app timer for temperature readings every 1 second
  app_timer_init();
  app_timer_create(&temp_timer, APP_TIMER_MODE_REPEATED, sensor_timer_callback);
  app_timer_start(temp_timer, 32768, NULL); // 32768 ticks = 1 second

  // Loop forever
  while (1) {
    // Don't put any code in here. Instead put periodic code in a callback using a timer.
    nrf_delay_ms(1000);
  }
}

