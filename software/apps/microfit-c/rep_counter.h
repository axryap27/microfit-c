// Dumbbell rep and set counter using accelerometer data

#pragma once

#include <stdint.h>
#include "adxl335.h"

#define TARGET_REPS 8
#define TARGET_SETS 3

// Initialize rep counter state
void rep_counter_init(void);

// Process a new accelerometer sample and detect reps
// Call this at a regular interval (e.g. 100Hz)
void rep_counter_update(adxl335_measurement_t accel);

// Get current rep count within the current set
uint8_t rep_counter_get_reps(void);

// Get current set number (1-based)
uint8_t rep_counter_get_set(void);

// Returns true when all sets are complete
bool rep_counter_is_done(void);

// Returns true during the rest period between sets
bool rep_counter_is_resting(void);

// Get seconds remaining in rest period
uint8_t rep_counter_get_rest_seconds(void);

// Reset everything back to set 1, rep 0
void rep_counter_reset(void);
