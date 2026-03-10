// Non-volatile memory (24LC32 EEPROM) app
//
// Reads a value, prints it, increments it, and writes it back every second.
// Persists across resets and power cycles.

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "app_timer.h"
#include "nrf_delay.h"
#include "nrf_twi_mngr.h"

#include "microbit_v2.h"
#include "24lc32.h"

// I2C manager
NRF_TWI_MNGR_DEF(twi_mngr_instance, 1, 0);

// App timer
APP_TIMER_DEF(nvm_timer);

// EEPROM address to use for our counter
static const uint16_t COUNTER_ADDR = 0x0000;

static void nvm_timer_callback(void* _unused) {
  // Read current value
  uint8_t value = nvm_read_byte(COUNTER_ADDR);
  printf("Read value: %d\n", value);

  // Increment and write back
  value++;
  nvm_write_byte(COUNTER_ADDR, value);
  printf("Wrote value: %d\n\n", value);
}

int main(void) {
  printf("Board started!\n");

  // Initialize I2C peripheral and driver
  nrf_drv_twi_config_t i2c_config = NRF_DRV_TWI_DEFAULT_CONFIG;
  i2c_config.scl = I2C_QWIIC_SCL;
  i2c_config.sda = I2C_QWIIC_SDA;
  i2c_config.frequency = NRF_DRV_TWI_FREQ_100K;
  i2c_config.interrupt_priority = 0;
  nrf_twi_mngr_init(&twi_mngr_instance, &i2c_config);

  // Initialize the EEPROM driver
  nvm_init(&twi_mngr_instance);

  // Initialize and start app timer (1 second interval)
  app_timer_init();
  app_timer_create(&nvm_timer, APP_TIMER_MODE_REPEATED, nvm_timer_callback);
  app_timer_start(nvm_timer, 32768, NULL);

  // Loop forever
  while (1) {
    nrf_delay_ms(1000);
  }
}
