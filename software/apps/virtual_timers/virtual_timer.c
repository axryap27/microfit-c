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

// Helper function: Update CC[0] to the soonest expiring timer
// Call this after any modification to the linked list
// Does nothing if the list is empty
static void update_compare_register(void) {
  node_t* first = list_get_first();
  if (first != NULL) {
    NRF_TIMER4->CC[0] = first->timer_value;
  }
}

// Helper function: Handle all expired timers
// Processes and fires callbacks for any timers that have already expired
// This handles the edge case where timers fire very close together
// Note: This function handles its own interrupt disable/enable for safety
static void handle_expired_timers(void) {
  while (1) {
    __disable_irq();

    node_t* first = list_get_first();
    uint32_t current_time = read_timer();

    // Check if there's an expired timer at the head
    if (first == NULL || first->timer_value > current_time) {
      // No expired timers, update CC[0] and exit
      update_compare_register();
      __enable_irq();
      return;
    }

    // Remove the expired timer from the list
    node_t* timer_node = list_remove_first();

    // Save timer info before potentially re-inserting
    bool is_repeated = timer_node->repeated;
    uint32_t interval = timer_node->microseconds;
    virtual_timer_callback_t callback = timer_node->callback;

    if (is_repeated) {
      // Update expiration time and re-insert into the list
      // Do this BEFORE enabling interrupts to maintain list consistency
      timer_node->timer_value += interval;
      list_insert_sorted(timer_node);
    }

    // Update CC[0] before enabling interrupts
    update_compare_register();

    __enable_irq();

    // Call the callback with interrupts enabled
    // (callbacks might take a while and shouldn't block other interrupts)
    callback();

    // Free the node if it was one-shot (after callback completes)
    if (!is_repeated) {
      free(timer_node);
    }

    // Loop to check for more expired timers
  }
}

// This is the interrupt handler that fires on a compare event
void TIMER4_IRQHandler(void)
{
  // This should always be the first line of the interrupt handler!
  // It clears the event so that it doesn't happen again
  NRF_TIMER4->EVENTS_COMPARE[0] = 0;

  // Capture timer value at start of handler
  irq_start_time = read_timer();

  // Handle all expired timers (including ones firing close together)
  // This function checks for NULL and handles all edge cases
  // Note: We don't need to disable interrupts here because
  // interrupts don't preempt themselves (same priority)
  handle_expired_timers();

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

  // Disable interrupts to prevent concurrency issues with linked list
  __disable_irq();

  // Insert the timer into the sorted linked list
  list_insert_sorted(timer_node);

  // Set CC[0] to the soonest expiring timer (head of list)
  update_compare_register();

  // Re-enable interrupts
  __enable_irq();

  // Handle any timers that may have already expired
  // (edge case: timer set for very short duration or CC was already past)
  handle_expired_timers();

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
  // Cast the timer_id back to a node pointer
  node_t* timer_node = (node_t*)timer_id;

  // Check for NULL pointer
  if (timer_node == NULL) {
    return;
  }

  // Disable interrupts to prevent concurrency issues with linked list
  __disable_irq();

  // Remove the timer from the linked list
  list_remove(timer_node);

  // Update CC[0] to the soonest expiring timer (may now be different)
  update_compare_register();

  // Re-enable interrupts
  __enable_irq();

  // Free the memory for the canceled timer
  free(timer_node);
}
