// Interrupt app
//
// Trigger GPIO and Software interrupts

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "nrf.h"
#include "nrf_delay.h"

#include "microbit_v2.h"

// Initialize software interrupts
void software_interrupt_init(void) {
  NRF_EGU1->INTENSET = 0x1;
  NVIC_EnableIRQ(SWI1_EGU1_IRQn);
}

// Trigger a software interrupt
void software_interrupt_trigger(void) {
  NRF_EGU1->TASKS_TRIGGER[0] = 1;
}

void SWI1_EGU1_IRQHandler(void) {
  // Clear interrupt event
  NRF_EGU1->EVENTS_TRIGGERED[0] = 0;
  printf("SWI Start!\n");
  for (int i = 0; i < 5; i++){
    printf("Stepping through\n");
    nrf_delay_ms(1000);
  }
  printf("SWI End!\n");
}

void GPIOTE_IRQHandler(void) {
  // Clear interrupt event
  NRF_GPIOTE->EVENTS_IN[0] = 0;

  printf("Button A pressed! (GPIOTE interrupt)\n");
}

int main(void) {
  printf("Board started!\n");


  NRF_GPIOTE->CONFIG[0] = (1 << 0) |      // MODE: Event
                          (14 << 8) |     // PSEL: Pin 14
                          (0 << 13) |     // PORT: Port 0
                          (2 << 16);      // POLARITY: HiToLo

  // Enable interrupt for IN[0] event (channel 0)
  NRF_GPIOTE->INTENSET = (1 << 0);

  // Enable GPIOTE interrupt in NVIC and set priority
  NVIC_SetPriority(GPIOTE_IRQn, 1);
  NVIC_EnableIRQ(GPIOTE_IRQn);

  NVIC_SetPriority(SWI1_EGU1_IRQn, 7);



  // After the GPIOTE is working, next trigger a software interrupt
  // Use the software_interupt_* functions defined above
  // Add code here
  software_interrupt_init();
  software_interrupt_trigger();



  // loop forever
  while (1) {
    printf("Looping\n");
    nrf_delay_ms(1000);
  }
}

