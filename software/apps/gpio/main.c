// GPIO app
//
// Uses MMIO to control GPIO pins

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "nrf.h"
#include "nrf_delay.h"

#include "microbit_v2.h"
#include "gpio.h"

int main(void) {
  printf("Board started!\n");

  // Turn on all LEDs on the back of the Microbit
  // Add code here
    const uint8_t rows[5] = { 21, 22, 15, 24, 19 };    
  const uint8_t cols[5] = { 28, 11, 31, 37, 30 };    

  for (int i = 0; i < 5; i++) {
    gpio_config(rows[i], OUTPUT);
    gpio_config(cols[i], OUTPUT);
  }
  for (int i = 0; i < 5; i++) {
    gpio_set(rows[i]);
    gpio_clear(cols[i]);
  }

  
  // Control LED with raw MMIO
  // Microphone LED is P0.20 and active high
  // Add code here

  *(volatile uint32_t*)0x50000518 = (1u << 20);
  *(volatile uint32_t*)0x50000508 = (1u << 20);


  // loop forever
  printf("Looping\n");
  while (1) {

    // Control LED with buttons
    // Button A is P0.14 and active low
    // Button B is P0.23 and active low
    // Add code here
    gpio_config(14, INPUT);
    gpio_config(23, INPUT);

    if (!gpio_read(14)) {
      // Button A pressed: turn mic LED on
      *(volatile uint32_t*)0x50000508 = (1u << 20);   // OUTSET P0.20
    }
    if (!gpio_read(23)) {
      // Button B pressed: turn mic LED off
      *(volatile uint32_t*)0x5000050C = (1u << 20);   // OUTCLR P0.20
    }

    nrf_delay_ms(100);
  }
}

