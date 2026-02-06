// Clock time implementation

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nrf.h"
#include "nrf_delay.h"

#include "clock_time.h"
#include "virtual_timer.h"

// Get the timestamp when compiling. Ignore the warning about this
#pragma GCC diagnostic ignored "-Wdate-time"
static const char* initial_timestr = __TIME__;

// Current sense of time, automatically updated
static clock_time_t current_time = {0};

// Callback function that runs every second to update the clock
static void clock_tick_callback(void) {
  // Increment seconds
  current_time.seconds++;

  // Handle overflow: seconds -> minutes
  if (current_time.seconds >= 60) {
    current_time.seconds = 0;
    current_time.minutes++;

    // Handle overflow: minutes -> hours
    if (current_time.minutes >= 60) {
      current_time.minutes = 0;
      current_time.hours++;

      // Handle overflow: hours wrap around at 24
      if (current_time.hours >= 24) {
        current_time.hours = 0;
      }
    }
  }
}

// WARNING: MUST be initialized after the virtual timer library
void clock_time_init(void) {
  printf("Initial timestr is: %s\n", initial_timestr); // debugging, you can remove this

  // Parse the __TIME__ string which is in format "HH:MM:SS"
  // Use strtol for safe string to number conversion
  char timestr_copy[9];
  strncpy(timestr_copy, initial_timestr, 8);
  timestr_copy[8] = '\0';

  // Parse hours (characters 0-1)
  char* endptr;
  current_time.hours = (uint8_t)strtol(&timestr_copy[0], &endptr, 10);

  // Parse minutes (characters 3-4, skip the ':' at position 2)
  current_time.minutes = (uint8_t)strtol(&timestr_copy[3], &endptr, 10);

  // Parse seconds (characters 6-7, skip the ':' at position 5)
  current_time.seconds = (uint8_t)strtol(&timestr_copy[6], &endptr, 10);

  // Start a repeating timer that fires every 1 second (1,000,000 microseconds)
  virtual_timer_start_repeated(1000000, clock_tick_callback);
}

clock_time_t clock_time_get(void) {
  return current_time;
}

