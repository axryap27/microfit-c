// MAX30102 pulse oximeter / heart rate sensor driver

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "nrf_twi_mngr.h"

// I2C address
static const uint8_t MAX30102_ADDRESS = 0x57;

// Register addresses
typedef enum {
  MAX30102_INT_STATUS_1    = 0x00,
  MAX30102_INT_STATUS_2    = 0x01,
  MAX30102_INT_ENABLE_1    = 0x02,
  MAX30102_INT_ENABLE_2    = 0x03,
  MAX30102_FIFO_WR_PTR     = 0x04,
  MAX30102_FIFO_OVF_CTR    = 0x05,
  MAX30102_FIFO_RD_PTR     = 0x06,
  MAX30102_FIFO_DATA       = 0x07,
  MAX30102_FIFO_CONFIG     = 0x08,
  MAX30102_MODE_CONFIG     = 0x09,
  MAX30102_SPO2_CONFIG     = 0x0A,
  MAX30102_LED1_PA         = 0x0C,
  MAX30102_LED2_PA         = 0x0D,
  MAX30102_MULTI_LED_1     = 0x11,
  MAX30102_MULTI_LED_2     = 0x12,
  MAX30102_TEMP_INT        = 0x1F,
  MAX30102_TEMP_FRAC       = 0x20,
  MAX30102_TEMP_CONFIG     = 0x21,
  MAX30102_REV_ID          = 0xFE,
  MAX30102_PART_ID         = 0xFF,
} max30102_reg_t;

// Initialize the MAX30102 sensor
void max30102_init(const nrf_twi_mngr_t* i2c);

// Read available FIFO samples and process for heart rate
// Call this periodically (e.g. 10-20Hz is fine since FIFO buffers)
void max30102_update(void);

// Get current heart rate in BPM (0 if no valid reading)
uint16_t max30102_read_bpm(void);

// Get latest raw IR value (useful for debugging)
uint32_t max30102_get_ir(void);

// Check if sensor is connected (Part ID == 0x15)
bool max30102_is_connected(void);
