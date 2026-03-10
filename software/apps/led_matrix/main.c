// LED Matrix app
//
// Display messages on the LED matrix

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "nrf_delay.h"

#include "led_matrix.h"
#include "microbit_v2.h"
#include "snake_game.h"

int main(void) {
  printf("Board started!\n");

  // initialize LED matrix driver and snake game
  led_matrix_init();
  snake_game_init();

  // game loop: refresh display rapidly, advance game every ~500ms
  uint32_t tick = 0;
  while (1) {
    led_matrix_refresh();  // strobe all rows once (~5ms)
    tick++;
    if (tick >= 100) {     // advance game state every ~500ms
      snake_game_advance_state();
      tick = 0;
    }
  }
}

