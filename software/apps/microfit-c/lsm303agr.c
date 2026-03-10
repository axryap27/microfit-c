// LSM303AGR driver for Microbit_v2
//
// Initializes sensor and communicates over I2C
// Capable of reading temperature, acceleration, and magnetic field strength

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

#include "lsm303agr.h"
#include "nrf_delay.h"

// Pointer to an initialized I2C instance to use for transactions
static const nrf_twi_mngr_t* i2c_manager = NULL;

// Helper function to perform a 1-byte I2C read of a given register
static uint8_t i2c_reg_read(uint8_t i2c_addr, uint8_t reg_addr) {
  uint8_t rx_buf = 0;
  nrf_twi_mngr_transfer_t const read_transfer[] = {
    NRF_TWI_MNGR_WRITE(i2c_addr, &reg_addr, 1, NRF_TWI_MNGR_NO_STOP),
    NRF_TWI_MNGR_READ(i2c_addr, &rx_buf, 1, 0),
  };
  ret_code_t result = nrf_twi_mngr_perform(i2c_manager, NULL, read_transfer, 2, NULL);
  if (result != NRF_SUCCESS) {
    printf("I2C transaction failed! Error: %lX\n", result);
  }
  return rx_buf;
}

// Helper function to perform a 1-byte I2C write of a given register
static void i2c_reg_write(uint8_t i2c_addr, uint8_t reg_addr, uint8_t data) {
  uint8_t buf[2] = {reg_addr, data};
  nrf_twi_mngr_transfer_t const write_transfer[] = {
    NRF_TWI_MNGR_WRITE(i2c_addr, buf, 2, 0),
  };
  ret_code_t result = nrf_twi_mngr_perform(i2c_manager, NULL, write_transfer, 1, NULL);
  if (result != NRF_SUCCESS) {
    printf("I2C write failed! Error: %lX\n", result);
  }
}

void lsm303agr_init(const nrf_twi_mngr_t* i2c) {
  i2c_manager = i2c;

  // Reboot accelerometer
  i2c_reg_write(LSM303AGR_ACC_ADDRESS, CTRL_REG5_A, 0x80);
  nrf_delay_ms(100);

  // Enable Block Data Update
  i2c_reg_write(LSM303AGR_ACC_ADDRESS, CTRL_REG4_A, 0x80);

  // Configure accelerometer at 100Hz, normal mode (10-bit), enable all axes
  i2c_reg_write(LSM303AGR_ACC_ADDRESS, CTRL_REG1_A, 0x57);

  uint8_t acc_who_am_i = i2c_reg_read(LSM303AGR_ACC_ADDRESS, WHO_AM_I_A);
  printf("Accelerometer WHO_AM_I: 0x%02X\n", acc_who_am_i);

  // Reboot magnetometer
  i2c_reg_write(LSM303AGR_MAG_ADDRESS, CFG_REG_A_M, 0x40);
  nrf_delay_ms(100);

  // Enable Block Data Update
  i2c_reg_write(LSM303AGR_MAG_ADDRESS, CFG_REG_C_M, 0x10);

  // Configure magnetometer at 100Hz, continuous mode
  i2c_reg_write(LSM303AGR_MAG_ADDRESS, CFG_REG_A_M, 0x0C);

  uint8_t mag_who_am_i = i2c_reg_read(LSM303AGR_MAG_ADDRESS, WHO_AM_I_M);
  printf("Magnetometer WHO_AM_I: 0x%02X\n", mag_who_am_i);

  // Enable temperature sensor
  i2c_reg_write(LSM303AGR_ACC_ADDRESS, TEMP_CFG_REG_A, 0xC0);
  nrf_delay_ms(100);
}

float lsm303agr_read_temperature(void) {
  uint8_t temp_l = i2c_reg_read(LSM303AGR_ACC_ADDRESS, OUT_TEMP_L_A);
  uint8_t temp_h = i2c_reg_read(LSM303AGR_ACC_ADDRESS, OUT_TEMP_H_A);
  int16_t temp_raw = (int16_t)((temp_h << 8) | temp_l);
  return ((float)temp_raw * (1.0 / 256.0)) + 25.0;
}

lsm303agr_measurement_t lsm303agr_read_accelerometer(void) {
  uint8_t x_l = i2c_reg_read(LSM303AGR_ACC_ADDRESS, OUT_X_L_A);
  uint8_t x_h = i2c_reg_read(LSM303AGR_ACC_ADDRESS, OUT_X_H_A);
  uint8_t y_l = i2c_reg_read(LSM303AGR_ACC_ADDRESS, OUT_Y_L_A);
  uint8_t y_h = i2c_reg_read(LSM303AGR_ACC_ADDRESS, OUT_Y_H_A);
  uint8_t z_l = i2c_reg_read(LSM303AGR_ACC_ADDRESS, OUT_Z_L_A);
  uint8_t z_h = i2c_reg_read(LSM303AGR_ACC_ADDRESS, OUT_Z_H_A);

  int16_t x_raw = ((int16_t)((x_h << 8) | x_l)) >> 6;
  int16_t y_raw = ((int16_t)((y_h << 8) | y_l)) >> 6;
  int16_t z_raw = ((int16_t)((z_h << 8) | z_l)) >> 6;

  lsm303agr_measurement_t measurement;
  measurement.x_axis = (float)x_raw * 3.9 / 1000.0;
  measurement.y_axis = (float)y_raw * 3.9 / 1000.0;
  measurement.z_axis = (float)z_raw * 3.9 / 1000.0;
  return measurement;
}

lsm303agr_measurement_t lsm303agr_read_magnetometer(void) {
  uint8_t x_l = i2c_reg_read(LSM303AGR_MAG_ADDRESS, OUTX_L_REG_M);
  uint8_t x_h = i2c_reg_read(LSM303AGR_MAG_ADDRESS, OUTX_H_REG_M);
  uint8_t y_l = i2c_reg_read(LSM303AGR_MAG_ADDRESS, OUTY_L_REG_M);
  uint8_t y_h = i2c_reg_read(LSM303AGR_MAG_ADDRESS, OUTY_H_REG_M);
  uint8_t z_l = i2c_reg_read(LSM303AGR_MAG_ADDRESS, OUTZ_L_REG_M);
  uint8_t z_h = i2c_reg_read(LSM303AGR_MAG_ADDRESS, OUTZ_H_REG_M);

  int16_t x_raw = (int16_t)((x_h << 8) | x_l);
  int16_t y_raw = (int16_t)((y_h << 8) | y_l);
  int16_t z_raw = (int16_t)((z_h << 8) | z_l);

  lsm303agr_measurement_t measurement;
  measurement.x_axis = (float)x_raw * 1.5 / 10.0;
  measurement.y_axis = (float)y_raw * 1.5 / 10.0;
  measurement.z_axis = (float)z_raw * 1.5 / 10.0;
  return measurement;
}
