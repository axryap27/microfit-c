// MAX30102 pulse oximeter / heart rate sensor driver
//
// Communicates over I2C, reads IR LED data from FIFO,
// and uses peak detection to compute BPM.

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "nrf_delay.h"
#include "max30102.h"

// BPM algorithm parameters
#define BPM_HISTORY_SIZE  10
#define MIN_BEAT_INTERVAL 300   // ms, max ~200 BPM
#define MAX_BEAT_INTERVAL 2000  // ms, min ~30 BPM
#define IR_THRESHOLD      50000 // minimum IR value to consider finger present
#define MIN_PEAK_AMP      200   // minimum peak-to-trough amplitude to count as beat

static const nrf_twi_mngr_t* i2c_manager = NULL;
static bool connected = false;

// BPM state
static uint16_t bpm;
static int32_t ibi_history[BPM_HISTORY_SIZE];
static uint8_t ibi_index;
static uint32_t last_ir;
static int32_t ir_filtered;       // low-pass filtered signal
static int32_t ir_filtered_prev;  // previous filtered value
static int32_t ir_peak;           // current peak value
static int32_t ir_trough;         // current trough value
static bool rising;
static uint32_t sample_count;
static uint32_t last_beat_sample;

// Helper: 1-byte I2C read
static uint8_t i2c_reg_read(uint8_t reg_addr) {
  uint8_t rx_buf = 0;
  nrf_twi_mngr_transfer_t const read_transfer[] = {
    NRF_TWI_MNGR_WRITE(MAX30102_ADDRESS, &reg_addr, 1, NRF_TWI_MNGR_NO_STOP),
    NRF_TWI_MNGR_READ(MAX30102_ADDRESS, &rx_buf, 1, 0),
  };
  ret_code_t result = nrf_twi_mngr_perform(i2c_manager, NULL, read_transfer, 2, NULL);
  if (result != NRF_SUCCESS) {
    printf("MAX30102 read failed! Error: %lX\n", result);
  }
  return rx_buf;
}

// Helper: 1-byte I2C write
static void i2c_reg_write(uint8_t reg_addr, uint8_t data) {
  uint8_t buf[2] = {reg_addr, data};
  nrf_twi_mngr_transfer_t const write_transfer[] = {
    NRF_TWI_MNGR_WRITE(MAX30102_ADDRESS, buf, 2, 0),
  };
  ret_code_t result = nrf_twi_mngr_perform(i2c_manager, NULL, write_transfer, 1, NULL);
  if (result != NRF_SUCCESS) {
    printf("MAX30102 write failed! Error: %lX\n", result);
  }
}

// Helper: read 6 bytes from FIFO (one sample = 3 bytes red + 3 bytes IR)
static void fifo_read_sample(uint32_t* red, uint32_t* ir) {
  uint8_t reg = MAX30102_FIFO_DATA;
  uint8_t buf[6] = {0};
  nrf_twi_mngr_transfer_t const read_transfer[] = {
    NRF_TWI_MNGR_WRITE(MAX30102_ADDRESS, &reg, 1, NRF_TWI_MNGR_NO_STOP),
    NRF_TWI_MNGR_READ(MAX30102_ADDRESS, buf, 6, 0),
  };
  ret_code_t result = nrf_twi_mngr_perform(i2c_manager, NULL, read_transfer, 2, NULL);
  if (result != NRF_SUCCESS) {
    *red = 0;
    *ir = 0;
    return;
  }
  // Each value is 18-bit, stored in 3 bytes (MSB first), top 2 bits unused
  *red = ((uint32_t)(buf[0] & 0x03) << 16) | ((uint32_t)buf[1] << 8) | buf[2];
  *ir  = ((uint32_t)(buf[3] & 0x03) << 16) | ((uint32_t)buf[4] << 8) | buf[5];
}

// Process one IR sample for beat detection
static void process_sample(uint32_t ir_val) {
  last_ir = ir_val;
  sample_count++;

  // Ignore if no finger detected
  if (ir_val < IR_THRESHOLD) {
    bpm = 0;
    rising = false;
    ir_filtered = (int32_t)ir_val;
    ir_filtered_prev = ir_filtered;
    return;
  }

  // Low-pass filter: smooth out noise (alpha ≈ 0.25)
  ir_filtered = ir_filtered - (ir_filtered >> 2) + ((int32_t)ir_val >> 2);

  // Track peak and trough
  if (ir_filtered > ir_peak) {
    ir_peak = ir_filtered;
  }
  if (ir_filtered < ir_trough) {
    ir_trough = ir_filtered;
  }

  // Detect transition from rising to falling (peak found)
  if (ir_filtered < ir_filtered_prev && rising) {
    int32_t amplitude = ir_peak - ir_trough;

    // Only count as beat if amplitude is significant
    if (amplitude > MIN_PEAK_AMP) {
      uint32_t interval_samples = sample_count - last_beat_sample;
      // 100Hz base / 4x averaging = 25Hz effective, each sample ≈ 40ms
      uint32_t interval_ms = interval_samples * 40;

      if (interval_ms > MIN_BEAT_INTERVAL && interval_ms < MAX_BEAT_INTERVAL) {
        ibi_history[ibi_index] = interval_ms;
        ibi_index = (ibi_index + 1) % BPM_HISTORY_SIZE;

        // Average IBI for stable BPM
        int32_t total = 0;
        for (int i = 0; i < BPM_HISTORY_SIZE; i++) {
          total += ibi_history[i];
        }
        int32_t avg_ibi = total / BPM_HISTORY_SIZE;
        if (avg_ibi > 0) {
          bpm = 60000 / avg_ibi;
        }

        last_beat_sample = sample_count;
      } else if (interval_ms >= MAX_BEAT_INTERVAL) {
        last_beat_sample = sample_count;
      }
    }

    // Reset trough for next cycle
    ir_trough = ir_filtered;
    rising = false;
  }

  if (ir_filtered > ir_filtered_prev) {
    rising = true;
    // Reset peak for next cycle when starting to rise
    if (!rising) {
      ir_peak = ir_filtered;
    }
  }

  ir_filtered_prev = ir_filtered;
}

void max30102_init(const nrf_twi_mngr_t* i2c) {
  i2c_manager = i2c;

  // Reset
  i2c_reg_write(MAX30102_MODE_CONFIG, 0x40);
  nrf_delay_ms(100);

  // Check part ID
  uint8_t part_id = i2c_reg_read(MAX30102_PART_ID);
  printf("MAX30102 Part ID: 0x%02X\n", part_id);
  if (part_id == 0x15) {
    printf("MAX30102 detected!\n");
    connected = true;
  } else {
    printf("MAX30102 not found (expected 0x15)\n");
    connected = false;
    return;
  }

  // FIFO config: 4 sample averaging, FIFO rollover enabled
  i2c_reg_write(MAX30102_FIFO_CONFIG, 0x50);

  // SpO2 mode (Red + IR LEDs)
  i2c_reg_write(MAX30102_MODE_CONFIG, 0x03);

  // SpO2 config: ADC range 4096, 100 samples/sec, 411us pulse width
  i2c_reg_write(MAX30102_SPO2_CONFIG, 0x27);

  // LED pulse amplitudes (~7mA each, gentle for finger)
  i2c_reg_write(MAX30102_LED1_PA, 0x24);
  i2c_reg_write(MAX30102_LED2_PA, 0x24);

  // Clear FIFO pointers
  i2c_reg_write(MAX30102_FIFO_WR_PTR, 0x00);
  i2c_reg_write(MAX30102_FIFO_OVF_CTR, 0x00);
  i2c_reg_write(MAX30102_FIFO_RD_PTR, 0x00);

  // Init BPM state
  bpm = 0;
  ibi_index = 0;
  last_ir = 0;
  ir_filtered = 0;
  ir_filtered_prev = 0;
  ir_peak = 0;
  ir_trough = 0;
  rising = false;
  sample_count = 0;
  last_beat_sample = 0;
  for (int i = 0; i < BPM_HISTORY_SIZE; i++) {
    ibi_history[i] = 800; // ~75 BPM default
  }
}

void max30102_update(void) {
  if (!connected) return;

  // Read number of available samples
  uint8_t wr_ptr = i2c_reg_read(MAX30102_FIFO_WR_PTR);
  uint8_t rd_ptr = i2c_reg_read(MAX30102_FIFO_RD_PTR);
  int num_samples = (wr_ptr - rd_ptr) & 0x1F;

  // Read and process each sample
  for (int i = 0; i < num_samples; i++) {
    uint32_t red, ir;
    fifo_read_sample(&red, &ir);
    process_sample(ir);
  }
}

uint16_t max30102_read_bpm(void) {
  return bpm;
}

uint32_t max30102_get_ir(void) {
  return last_ir;
}

bool max30102_is_connected(void) {
  return connected;
}
