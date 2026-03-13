// ADXL335 analog 3-axis accelerometer driver
//
// Uses nRF52 SAADC to read analog voltages from X, Y, Z pins.
// At 3.3V supply:
//   0g output ≈ 1.65V (midpoint)
//   Sensitivity ≈ 330mV/g
//
// Pins: X=EDGE_P0, Y=EDGE_P1, Z=EDGE_P2
//
// NOTE: Currently commented out - using internal LSM303AGR instead.
// Uncomment all code below for ADXL335 submission.

#include <stdint.h>
#include <stdio.h>

// #include "nrf_drv_saadc.h"
// #include "microbit_v2.h"
#include "adxl335.h"

// // ADC channels
// #define CH_X 0
// #define CH_Y 1
// #define CH_Z 2
//
// // ADXL335 at 3.3V supply
// // 0g voltage = VCC/2 = 1.65V
// // Sensitivity = 330mV/g (typical at 3.3V)
// //
// // 10-bit ADC with 3.6V reference (nRF52 internal):
// //   ADC value = (Vin / 3.6V) * 1024
// //   At 0g: ~469
// //   Per g:  (0.330V / 3.6V) * 1024 ≈ 93.9 counts/g
// #define ADC_ZERO_G   469.0f
// #define ADC_PER_G    93.9f
//
// // SAADC callback (required but unused in blocking mode)
// static void saadc_callback(nrf_drv_saadc_evt_t const* p_event) {
//   (void)p_event;
// }
//
// void adxl335_init(void) {
//   nrf_drv_saadc_config_t saadc_config = NRF_DRV_SAADC_DEFAULT_CONFIG;
//   saadc_config.resolution = NRF_SAADC_RESOLUTION_10BIT;
//   nrf_drv_saadc_init(&saadc_config, saadc_callback);
//
//   // Channel 0: X axis on EDGE_P0
//   nrf_saadc_channel_config_t ch_x_config =
//     NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(EDGE_P0_ANA);
//   nrf_drv_saadc_channel_init(CH_X, &ch_x_config);
//
//   // Channel 1: Y axis on EDGE_P1
//   nrf_saadc_channel_config_t ch_y_config =
//     NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(EDGE_P1_ANA);
//   nrf_drv_saadc_channel_init(CH_Y, &ch_y_config);
//
//   // Channel 2: Z axis on EDGE_P2
//   nrf_saadc_channel_config_t ch_z_config =
//     NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(EDGE_P2_ANA);
//   nrf_drv_saadc_channel_init(CH_Z, &ch_z_config);
//
//   printf("ADXL335 initialized (P0=X, P1=Y, P2=Z)\n");
// }
//
// adxl335_measurement_t adxl335_read_accelerometer(void) {
//   nrf_saadc_value_t x_raw = 0, y_raw = 0, z_raw = 0;
//
//   nrf_drv_saadc_sample_convert(CH_X, &x_raw);
//   nrf_drv_saadc_sample_convert(CH_Y, &y_raw);
//   nrf_drv_saadc_sample_convert(CH_Z, &z_raw);
//
//   // Clamp negatives (SAADC can return small negatives)
//   if (x_raw < 0) x_raw = 0;
//   if (y_raw < 0) y_raw = 0;
//   if (z_raw < 0) z_raw = 0;
//
//   adxl335_measurement_t m;
//   m.x_axis = ((float)x_raw - ADC_ZERO_G) / ADC_PER_G;
//   m.y_axis = ((float)y_raw - ADC_ZERO_G) / ADC_PER_G;
//   m.z_axis = ((float)z_raw - ADC_ZERO_G) / ADC_PER_G;
//   return m;
// }
