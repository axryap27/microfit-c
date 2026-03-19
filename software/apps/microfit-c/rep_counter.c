// Dumbbell rep counter implementation
//
// Uses accelerometer magnitude to detect reps via peak detection.
// Tracks reps within a set and auto-advances to the next set
// when TARGET_REPS is reached.

#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "rep_counter.h"

// Rep detection parameters (tuned for dumbbell curls / presses)
#define REP_THRESHOLD     0.35  // g above average to count as a rep
#define MIN_REP_INTERVAL  50    // minimum samples between reps (~500ms at 100Hz)
#define AVG_WINDOW        32    // samples for moving average (wider window for slower motion)
#define REST_SAMPLES      1500  // 15 seconds at 100Hz

static uint8_t rep_count;
static uint8_t set_count;       // 1-based current set
static bool    done;

static float   mag_history[AVG_WINDOW];
static uint8_t history_idx;
static float   mag_avg;
static uint32_t samples_since_rep;
static bool    above_threshold;
static bool    half_rep;         // toggles each peak; rep counts on every 2nd peak
static bool    resting;          // true during rest period between sets
static uint32_t rest_remaining;  // samples left in rest countdown

void rep_counter_init(void) {
  rep_count = 0;
  set_count = 1;
  done = false;
  history_idx = 0;
  mag_avg = 1.0; // ~1g at rest
  samples_since_rep = MIN_REP_INTERVAL;
  above_threshold = false;
  half_rep = false;
  resting = false;
  rest_remaining = 0;
  for (int i = 0; i < AVG_WINDOW; i++) {
    mag_history[i] = 1.0;
  }
}

void rep_counter_update(adxl335_measurement_t accel) {
  if (done) return;

  // During rest period, just count down
  if (resting) {
    if (rest_remaining > 0) {
      rest_remaining--;
    } else {
      resting = false;
      set_count++;
      rep_count = 0;
      half_rep = false;
      above_threshold = false;
      samples_since_rep = MIN_REP_INTERVAL;
    }
    return;
  }

  float x = accel.x_axis;
  float y = accel.y_axis;
  float z = accel.z_axis;

  float mag = sqrt(x * x + y * y + z * z);

  // Update moving average
  mag_avg -= mag_history[history_idx] / AVG_WINDOW;
  mag_history[history_idx] = mag;
  mag_avg += mag / AVG_WINDOW;
  history_idx = (history_idx + 1) % AVG_WINDOW;

  samples_since_rep++;

  // Detect rep: magnitude crosses above threshold then back below
  float diff = mag - mag_avg;

  if (diff > REP_THRESHOLD) {
    above_threshold = true;
  } else if (above_threshold && diff < 0) {
    if (samples_since_rep >= MIN_REP_INTERVAL) {
      half_rep = !half_rep;
      samples_since_rep = 0;

      // A full rep has two peaks (up + down); count on every 2nd peak
      if (!half_rep) {
        rep_count++;

        // When target reps reached, either finish or rest before next set
        if (rep_count >= TARGET_REPS) {
          if (set_count >= TARGET_SETS) {
            done = true;
          } else {
            resting = true;
            rest_remaining = REST_SAMPLES;
          }
        }
      }
    }
    above_threshold = false;
  }
}

uint8_t rep_counter_get_reps(void) {
  return rep_count;
}

uint8_t rep_counter_get_set(void) {
  return set_count;
}

bool rep_counter_is_done(void) {
  return done;
}

bool rep_counter_is_resting(void) {
  return resting;
}

uint8_t rep_counter_get_rest_seconds(void) {
  return (uint8_t)((rest_remaining + 99) / 100); // ceiling division, 100 samples = 1s
}

void rep_counter_reset(void) {
  rep_counter_init();
}
