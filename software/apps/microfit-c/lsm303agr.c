// LSM303AGR internal accelerometer driver (for step counting)

#include <stdint.h>
#include <stdio.h>

#include "nrf_delay.h"
#include "lsm303agr.h"

static const nrf_twi_mngr_t* i2c_manager = NULL;

static uint8_t i2c_reg_read(uint8_t i2c_addr, uint8_t reg_addr) {
  uint8_t rx_buf = 0;
  nrf_twi_mngr_transfer_t const read_transfer[] = {
    NRF_TWI_MNGR_WRITE(i2c_addr, &reg_addr, 1, NRF_TWI_MNGR_NO_STOP),
    NRF_TWI_MNGR_READ(i2c_addr, &rx_buf, 1, 0),
  };
  nrf_twi_mngr_perform(i2c_manager, NULL, read_transfer, 2, NULL);
  return rx_buf;
}

static void i2c_reg_write(uint8_t i2c_addr, uint8_t reg_addr, uint8_t data) {
  uint8_t buf[2] = {reg_addr, data};
  nrf_twi_mngr_transfer_t const write_transfer[] = {
    NRF_TWI_MNGR_WRITE(i2c_addr, buf, 2, 0),
  };
  nrf_twi_mngr_perform(i2c_manager, NULL, write_transfer, 1, NULL);
}

void lsm303agr_init(const nrf_twi_mngr_t* i2c) {
  i2c_manager = i2c;

  i2c_reg_write(LSM303AGR_ACC_ADDRESS, CTRL_REG5_A, 0x80);
  nrf_delay_ms(100);

  i2c_reg_write(LSM303AGR_ACC_ADDRESS, CTRL_REG4_A, 0x80);
  i2c_reg_write(LSM303AGR_ACC_ADDRESS, CTRL_REG1_A, 0x57);

  uint8_t who = i2c_reg_read(LSM303AGR_ACC_ADDRESS, WHO_AM_I_A);
  printf("LSM303AGR WHO_AM_I: 0x%02X\n", who);
}

adxl335_measurement_t lsm303agr_read_accelerometer(void) {
  uint8_t x_l = i2c_reg_read(LSM303AGR_ACC_ADDRESS, OUT_X_L_A);
  uint8_t x_h = i2c_reg_read(LSM303AGR_ACC_ADDRESS, OUT_X_H_A);
  uint8_t y_l = i2c_reg_read(LSM303AGR_ACC_ADDRESS, OUT_Y_L_A);
  uint8_t y_h = i2c_reg_read(LSM303AGR_ACC_ADDRESS, OUT_Y_H_A);
  uint8_t z_l = i2c_reg_read(LSM303AGR_ACC_ADDRESS, OUT_Z_L_A);
  uint8_t z_h = i2c_reg_read(LSM303AGR_ACC_ADDRESS, OUT_Z_H_A);

  int16_t x_raw = ((int16_t)((x_h << 8) | x_l)) >> 6;
  int16_t y_raw = ((int16_t)((y_h << 8) | y_l)) >> 6;
  int16_t z_raw = ((int16_t)((z_h << 8) | z_l)) >> 6;

  adxl335_measurement_t m;
  m.x_axis = (float)x_raw * 3.9 / 1000.0;
  m.y_axis = (float)y_raw * 3.9 / 1000.0;
  m.z_axis = (float)z_raw * 3.9 / 1000.0;
  return m;
}
