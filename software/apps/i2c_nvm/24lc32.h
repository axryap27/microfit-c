// 24LC32 I2C EEPROM driver

#pragma once

#include "nrf_twi_mngr.h"

// I2C address (A0, A1, A2 all grounded)
static const uint8_t NVM_ADDRESS = 0x50;

// Initialize the 24LC32 driver
//
// i2c - pointer to already initialized and enabled twim instance
void nvm_init(const nrf_twi_mngr_t* i2c);

// Read a single byte from the EEPROM
//
// address - 16-bit memory address to read from (0x0000 to 0x0FFF)
// Returns the byte at that address
uint8_t nvm_read_byte(uint16_t address);


// Write a single byte to the EEPROM
//
// address - 16-bit memory address to write to (0x0000 to 0x0FFF)
// data - byte to write
void nvm_write_byte(uint16_t address, uint8_t data);
