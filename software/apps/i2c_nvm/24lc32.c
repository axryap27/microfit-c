// 24LC32 I2C EEPROM driver
//
// Communicates over I2C to read/write single bytes

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "nrf_delay.h"
#include "24lc32.h"

// Pointer to an initialized I2C instance to use for transactions
static const nrf_twi_mngr_t* i2c_manager = NULL;

void nvm_init(const nrf_twi_mngr_t* i2c) {
  i2c_manager = i2c;
  // No particular initialization needed for this device
}

uint8_t nvm_read_byte(uint16_t address) {
  uint8_t addr_buf[2] = {(address >> 8) & 0xFF, address & 0xFF};
  uint8_t rx_buf = 0;

  nrf_twi_mngr_transfer_t const read_transfer[] = {
    NRF_TWI_MNGR_WRITE(NVM_ADDRESS, addr_buf, 2, NRF_TWI_MNGR_NO_STOP),
    NRF_TWI_MNGR_READ(NVM_ADDRESS, &rx_buf, 1, 0),
  };
  ret_code_t result = nrf_twi_mngr_perform(i2c_manager, NULL, read_transfer, 2, NULL);
  if (result != NRF_SUCCESS) {
    printf("NVM I2C read failed! Error: %lX\n", result);
  }

  return rx_buf;
}

void nvm_write_byte(uint16_t address, uint8_t data) {
  uint8_t buf[3] = {(address >> 8) & 0xFF, address & 0xFF, data};

  nrf_twi_mngr_transfer_t const write_transfer[] = {
    NRF_TWI_MNGR_WRITE(NVM_ADDRESS, buf, 3, 0),
  };
  ret_code_t result = nrf_twi_mngr_perform(i2c_manager, NULL, write_transfer, 1, NULL);
  if (result != NRF_SUCCESS) {
    printf("NVM I2C write failed! Error: %lX\n", result);
  }

  // EEPROM needs up to 5ms to complete the write cycle
  nrf_delay_ms(5);
}
