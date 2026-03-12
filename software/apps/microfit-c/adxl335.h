// ADXL335 analog 3-axis accelerometer driver
//
// Reads X, Y, Z from ADC pins (EDGE_P0, P1, P2)

#pragma once

#include <stdint.h>

// Measurement data type
typedef struct {
  float x_axis;
  float y_axis;
  float z_axis;
} adxl335_measurement_t;

// Initialize SAADC for 3-channel accelerometer reading
void adxl335_init(void);

// Read all three axes
// Returns measurements in g's
adxl335_measurement_t adxl335_read_accelerometer(void);
