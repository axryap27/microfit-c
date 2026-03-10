// Pulse Sensor driver
//
// Analog heart rate sensor using nRF52 SAADC
// Based on the PulseSensor algorithm by World Famous Electronics
// Samples at 500Hz, detects peaks in the optical signal to compute BPM

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "nrf_drv_saadc.h"
#include "microbit_v2.h"
#include "pulse_sensor.h"

// Number of IBI samples to average for BPM
#define IBI_HISTORY_SIZE 10

// State variables
static uint16_t signal_value;       // Current ADC reading
static uint16_t bpm;                // Beats per minute
static int16_t ibi_history[IBI_HISTORY_SIZE]; // Inter-beat intervals (ms)
static uint8_t ibi_index;

static int16_t threshold;           // Beat detection threshold
static int16_t peak;                // Peak value in current pulse wave
static int16_t trough;              // Trough value in current pulse wave

static uint32_t last_beat_time;     // Time of last beat (in 2ms sample ticks)
static uint32_t sample_counter;     // Counts 2ms samples since start

static bool pulse;                  // True during a pulse
static bool beat_detected;          // Flag: new beat just detected
static bool first_beat;             // Waiting for first beat
static bool second_beat;            // Waiting for second beat

// SAADC callback (required but we use blocking mode)
static void saadc_callback(nrf_drv_saadc_evt_t const* p_event) {
  (void)p_event;
}

void pulse_sensor_init(void) {
  // Initialize SAADC
  nrf_drv_saadc_config_t saadc_config = NRF_DRV_SAADC_DEFAULT_CONFIG;
  saadc_config.resolution = NRF_SAADC_RESOLUTION_10BIT;
  nrf_drv_saadc_init(&saadc_config, saadc_callback);

  // Configure channel 0 on EDGE_P0 analog input
  nrf_saadc_channel_config_t channel_config = NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(EDGE_P0_ANA);
  nrf_drv_saadc_channel_init(0, &channel_config);

  // Initialize state
  signal_value = 0;
  bpm = 0;
  ibi_index = 0;
  threshold = 512;  // Mid-range starting threshold
  peak = 512;
  trough = 512;
  last_beat_time = 0;
  sample_counter = 0;
  pulse = false;
  beat_detected = false;
  first_beat = true;
  second_beat = false;

  for (int i = 0; i < IBI_HISTORY_SIZE; i++) {
    ibi_history[i] = 600; // Default ~100 BPM
  }

  printf("Pulse sensor initialized on EDGE_P0\n");
}

void pulse_sensor_update(void) {
  // Read ADC value (blocking)
  nrf_saadc_value_t adc_value = 0;
  nrf_drv_saadc_sample_convert(0, &adc_value);
  if (adc_value < 0) adc_value = 0;
  signal_value = (uint16_t)adc_value;

  sample_counter++;
  uint32_t elapsed = sample_counter - last_beat_time; // in 2ms ticks
  uint32_t elapsed_ms = elapsed * 2;

  // Track trough (minimum)
  if (signal_value < threshold && signal_value < trough) {
    trough = signal_value;
  }

  // Track peak (maximum)
  if (signal_value > threshold && signal_value > peak) {
    peak = signal_value;
  }

  // Look for a heartbeat: signal rises above threshold
  if (elapsed_ms > 250) { // At least 250ms since last beat (max ~240 BPM)
    if (signal_value > threshold && !pulse) {
      pulse = true;
      beat_detected = true;

      int16_t ibi = elapsed_ms;

      if (second_beat) {
        second_beat = false;
        // Seed the IBI history with this interval
        for (int i = 0; i < IBI_HISTORY_SIZE; i++) {
          ibi_history[i] = ibi;
        }
      }

      if (first_beat) {
        first_beat = false;
        second_beat = true;
        last_beat_time = sample_counter;
        return; // Can't compute BPM with just one beat
      }

      // Compute running average of IBI
      int32_t running_total = 0;
      ibi_history[ibi_index] = ibi;
      ibi_index = (ibi_index + 1) % IBI_HISTORY_SIZE;
      for (int i = 0; i < IBI_HISTORY_SIZE; i++) {
        running_total += ibi_history[i];
      }
      running_total /= IBI_HISTORY_SIZE;

      if (running_total > 0) {
        bpm = 60000 / running_total;
      }

      last_beat_time = sample_counter;
    }
  }

  // Signal drops below threshold: pulse is over
  if (signal_value < threshold && pulse) {
    pulse = false;
    // Recalculate threshold as midpoint between peak and trough
    int16_t amplitude = peak - trough;
    threshold = (amplitude / 2) + trough;
    peak = threshold;
    trough = threshold;
  }

  // No beat for 2.5 seconds: reset
  if (elapsed_ms > 2500) {
    threshold = 512;
    peak = 512;
    trough = 512;
    bpm = 0;
    first_beat = true;
    second_beat = false;
    last_beat_time = sample_counter;
  }
}

uint16_t pulse_sensor_read_bpm(void) {
  return bpm;
}

uint16_t pulse_sensor_get_signal(void) {
  return signal_value;
}

bool pulse_sensor_saw_beat(void) {
  if (beat_detected) {
    beat_detected = false;
    return true;
  }
  return false;
}
