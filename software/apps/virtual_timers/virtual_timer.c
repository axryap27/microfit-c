// Virtual timer implementation

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "nrf.h"

#include "virtual_timer.h"
#include "virtual_timer_linked_list.h"

// Globals for tracking interrupt timing
volatile uint32_t irq_start_time = 0;
volatile uint32_t irq_end_time = 0;
volatile bool irq_timing_ready = false;

// This is the interrupt handler that fires on a compare event
void TIMER4_IRQHandler(void)
{
  // This should always be the first line of the interrupt handler!
  // It clears the event so that it doesn't happen again
  NRF_TIMER4->EVENTS_COMPARE[0] = 0;

  // Capture timer value at start of handler
  irq_start_time = read_timer();

  // Remove the first timer from the list and call its callback
  node_t* timer_node = list_remove_first();
  if (timer_node != NULL) {
    timer_node->callback();

    if (timer_node->repeated) {
      // Update expiration time and re-insert into the list
      timer_node->timer_value += timer_node->microseconds;
      list_insert_sorted(timer_node);

      // Update CC[0] to the next timer's expiration time
      node_t* first = list_get_first();
      NRF_TIMER4->CC[0] = first->timer_value;
    } else {
      free(timer_node);
    }
  }

  // Capture timer value at end of handler
  irq_end_time = read_timer();
  irq_timing_ready = true;
}

// Read the current value of the timer counter
uint32_t read_timer(void)
{

  // Capture the current timer value into CC[1]
  NRF_TIMER4->TASKS_CAPTURE[1] = 1;

  // Return the captured value
  return NRF_TIMER4->CC[1];
}

// Initialize the timers
void virtual_timer_init(void)
{
  // Place your timer initialization code here

  // Set 32-bit timer width
  NRF_TIMER4->BITMODE = 3;

  // Set prescaler to 4 (16 MHz / 2^4 = 1 MHz)
  NRF_TIMER4->PRESCALER = 4;

  // Clear the timer
  NRF_TIMER4->TASKS_CLEAR = 1;

  // Start the timer
  NRF_TIMER4->TASKS_START = 1;

  // Enable interrupt on CC[0] compare event (bit 16 in INTENSET)
  NRF_TIMER4->INTENSET = 1 << 16;

  // Enable TIMER4 interrupts in the NVIC
  NVIC_EnableIRQ(TIMER4_IRQn);
}

// This is a private helper function called from multiple public functions with different arguments.
// Starts a timer. This function is called for both one-shot and repeated timers
static uint32_t timer_start(uint32_t microseconds, virtual_timer_callback_t cb, bool repeated)
{

  // Allocate a new linked list node for this timer
  node_t* timer_node = (node_t*)malloc(sizeof(node_t));

  // Store the expiration time (current time + duration)
  timer_node->timer_value = read_timer() + microseconds;

  // Store the callback function
  timer_node->callback = cb;

  // Store whether this timer repeats and its interval
  timer_node->repeated = repeated;
  timer_node->microseconds = microseconds;

  // Insert the timer into the sorted linked list
  list_insert_sorted(timer_node);

  // Set CC[0] to the first timer's expiration time
  node_t* first = list_get_first();
  NRF_TIMER4->CC[0] = first->timer_value;

  // Return the pointer as a unique timer ID
  return (uint32_t)timer_node;
}

// You do not need to modify this function
// Instead, implement timer_start
uint32_t virtual_timer_start(uint32_t microseconds, virtual_timer_callback_t cb)
{
  return timer_start(microseconds, cb, false);
}

// You do not need to modify this function
// Instead, implement timer_start
uint32_t virtual_timer_start_repeated(uint32_t microseconds, virtual_timer_callback_t cb)
{
  return timer_start(microseconds, cb, true);
}

// Remove a timer by ID.
// Make sure you don't cause linked list consistency issues!
// Do not forget to free removed timers.
void virtual_timer_cancel(uint32_t timer_id)
{
}
