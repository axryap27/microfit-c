#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "nrf.h"

// Interrupt timing data (set by the ISR)
extern volatile uint32_t irq_start_time;
extern volatile uint32_t irq_end_time;
extern volatile bool irq_timing_ready;

// Type for the function pointer to call when the timer expires
// A virtual_timer_callback_t can be any function that takes no arguments and returns no value
// For example: `void my_timer_callback(void) { }`
typedef void (*virtual_timer_callback_t)(void);

// Read the current value of the hardware timer counter
// Returns the counter value
uint32_t read_timer(void);

// Initialize the timer peripheral
void virtual_timer_init(void);

// Start a one-shot timer that calls <cb> <microseconds> in the future
// Returns a unique timer_id
uint32_t virtual_timer_start(uint32_t microseconds, virtual_timer_callback_t cb);

// Start timer that repeatedly calls <cb> <microseconds> in the future
// Returns a unique timer_id
uint32_t virtual_timer_start_repeated(uint32_t microseconds, virtual_timer_callback_t cb);

// Takes a timer_id and cancels that timer such that it stops firing
void virtual_timer_cancel(uint32_t timer_id);
