// Virtual timers
//
// Uses a single hardware timer to create an unlimited supply of virtual
//  software timers

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "app_error.h"
#include "app_timer.h"
#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_serial.h"

#include "microbit_v2.h"
#include "virtual_timer.h"
#include "clock_time.h"

void led1_toggle() {
    nrf_gpio_pin_toggle(LED_ROW1);
}

void led2_toggle() {
    nrf_gpio_pin_toggle(LED_ROW2);
}

void led3_toggle() {
    nrf_gpio_pin_toggle(LED_ROW3);
}

int main(void) {
  printf("Board initialized!\n");

  // Configure Microbit pins
  nrf_gpio_pin_dir_set(LED_COL1, NRF_GPIO_PIN_DIR_OUTPUT);
  nrf_gpio_pin_dir_set(LED_ROW1, NRF_GPIO_PIN_DIR_OUTPUT);
  nrf_gpio_pin_dir_set(LED_ROW2, NRF_GPIO_PIN_DIR_OUTPUT);
  nrf_gpio_pin_dir_set(LED_ROW3, NRF_GPIO_PIN_DIR_OUTPUT);
  nrf_gpio_pin_clear(LED_COL1);
  nrf_gpio_pin_clear(LED_ROW1);
  nrf_gpio_pin_clear(LED_ROW2);
  nrf_gpio_pin_clear(LED_ROW3);

  // Initialize your timer library
  virtual_timer_init();

  // Initialize clock time library (must be after virtual_timer_init)
  clock_time_init();

  // Start three separate repeating timers for each LED
  // LED1: 1 second interval
  // LED2: 2 second interval
  // LED3: 4 second interval
  uint32_t timer1_id = virtual_timer_start_repeated(1000000, led1_toggle);   // 1 second
  uint32_t timer2_id = virtual_timer_start_repeated(2000000, led2_toggle);   // 2 seconds
  uint32_t timer3_id = virtual_timer_start_repeated(4000000, led3_toggle);   // 4 seconds

  printf("Started 3 timers: 1s, 2s, 4s\n");

  // loop forever
  while (1) {
    nrf_delay_ms(1000);
    uint32_t timer_value = read_timer();

    // Get and print the current clock time with zero-padded formatting
    clock_time_t now = clock_time_get();
    printf("Clock: %02d:%02d:%02d | Timer: %lu\n",
           now.hours, now.minutes, now.seconds, timer_value);

    // After 16 seconds, cancel the 2-second and 4-second timers
    if (timer_value >= 16000000 && timer2_id != 0) {
      printf("Canceling 2-second and 4-second timers\n");
      virtual_timer_cancel(timer2_id);
      virtual_timer_cancel(timer3_id);
      timer2_id = 0;  // Mark as canceled
      timer3_id = 0;
    }

    // After 20 seconds (16 + 4), cancel the 1-second timer
    if (timer_value >= 20000000 && timer1_id != 0) {
      printf("Canceling 1-second timer\n");
      virtual_timer_cancel(timer1_id);
      timer1_id = 0;  // Mark as canceled
    }

    if (irq_timing_ready) {
      printf("IRQ start time: %lu\n", irq_start_time);
      printf("IRQ end time:   %lu\n", irq_end_time);
      printf("IRQ duration:   %lu us\n", irq_end_time - irq_start_time);
      irq_timing_ready = false;
    }
  }
}

