// Step counter using accelerometer data

#pragma once

#include <stdint.h>
#include "adxl335.h"

// Initialize step counter state
void step_counter_init(void);

// Process a new accelerometer sample and detect steps
// Call this at a regular interval (e.g. 100Hz or from a timer callback)
void step_counter_update(adxl335_measurement_t accel);

// Get the current step count
uint32_t step_counter_get_steps(void);

// Reset the step count to zero
void step_counter_reset(void);
