// Step counter implementation
//
// Uses accelerometer magnitude to detect steps via peak detection.
// Algorithm: compute acceleration magnitude, track a moving average,
// and count a step when the magnitude crosses above a threshold
// after a minimum time gap (debounce).

#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "step_counter.h"

// Step detection parameters
#define STEP_THRESHOLD    0.25  // g above/below average to count as a step
#define MIN_STEP_INTERVAL 10    // minimum samples between steps (~100ms at 100Hz)
#define AVG_WINDOW        16    // number of samples for moving average

static uint32_t step_count;
static float mag_history[AVG_WINDOW];
static uint8_t history_idx;
static float mag_avg;
static uint32_t samples_since_step;
static bool above_threshold;

void step_counter_init(void) {
  step_count = 0;
  history_idx = 0;
  mag_avg = 1.0; // expect ~1g at rest
  samples_since_step = MIN_STEP_INTERVAL;
  above_threshold = false;
  for (int i = 0; i < AVG_WINDOW; i++) {
    mag_history[i] = 1.0;
  }
}

void step_counter_update(adxl335_measurement_t accel) {
  float x = accel.x_axis;
  float y = accel.y_axis;
  float z = accel.z_axis;

  // Compute acceleration magnitude
  float mag = sqrt(x * x + y * y + z * z);

  // Update moving average
  mag_avg -= mag_history[history_idx] / AVG_WINDOW;
  mag_history[history_idx] = mag;
  mag_avg += mag / AVG_WINDOW;
  history_idx = (history_idx + 1) % AVG_WINDOW;

  samples_since_step++;

  // Detect step: magnitude crosses above threshold, then back below
  float diff = mag - mag_avg;

  if (diff > STEP_THRESHOLD) {
    above_threshold = true;
  } else if (above_threshold && diff < 0) {
    // Crossed back below average after being above threshold
    if (samples_since_step >= MIN_STEP_INTERVAL) {
      step_count++;
      samples_since_step = 0;
    }
    above_threshold = false;
  }
}

uint32_t step_counter_get_steps(void) {
  return step_count;
}

void step_counter_reset(void) {
  step_count = 0;
}
