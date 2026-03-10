// Pulse Sensor driver
//
// Analog optical heart rate sensor read via SAADC
// Connect signal wire to EDGE_P0 (analog capable pin)

#pragma once

#include <stdbool.h>
#include <stdint.h>

// Initialize the pulse sensor ADC channel
void pulse_sensor_init(void);

// Process one ADC sample - call this at 500Hz
void pulse_sensor_update(void);

// Get the current heart rate in BPM
// Returns 0 if no valid reading yet
uint16_t pulse_sensor_read_bpm(void);

// Get the raw signal value (0-1023)
uint16_t pulse_sensor_get_signal(void);

// Check if a beat was just detected (resets after reading)
bool pulse_sensor_saw_beat(void);
