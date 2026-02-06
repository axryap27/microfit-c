#include "gpio.h"
#include <stdio.h>


#define GPIO_P0_BASE 0x50000000u
#define GPIO_P1_BASE 0x50000300u

typedef struct{
  // Step 3:
  // Add register definitions here
  uint32_t OUT;
  uint32_t OUTSET;
  uint32_t OUTCLR;
  uint32_t IN;
  uint32_t DIR;
  uint32_t DIRSET;
  uint32_t LATCH;
  uint32_t DETECTMODE;

  uint32_t RESERVED0[118];
  uint32_t PIN_CNF[32];
}gpio_reg_t;

static volatile gpio_reg_t* const gpio_ports[2] = {
  (volatile gpio_reg_t*)GPIO_P0_BASE,
  (volatile gpio_reg_t*)GPIO_P1_BASE
};

static inline volatile gpio_reg_t* gpio_port(uint8_t gpio_num) {
  return gpio_ports[gpio_num / 32];
}

static inline uint8_t gpio_pin(uint8_t gpio_num) {
  return gpio_num % 32;
}

// Inputs: 
//  gpio_num - gpio number 0-31 OR (32 + gpio number)
//  dir - gpio direction (INPUT, OUTPUT)
void gpio_config(uint8_t gpio_num, gpio_direction_t dir) {
  // Implement me
  // This function should configure the pin as an input/output
  // Hint: Use proper PIN_CNF instead of DIR
  volatile gpio_reg_t* port = gpio_port(gpio_num);
  uint8_t pin = gpio_pin(gpio_num);

  if (dir == OUTPUT) {
    port->PIN_CNF[pin] =
        (1 << 0) | 
        (0 << 1) |   
        (0 << 2);    
  } else {
    port->PIN_CNF[pin] =
        (0 << 0) |   
        (1 << 1) |  
        (0 << 2);    
  }
}

// Inputs: 
//  gpio_num - gpio number 0-31 OR (32 + gpio number)
void gpio_set(uint8_t gpio_num) {
  // Implement me
  // This function should make the pin high
  // It can assume that the pin has already been configured
  volatile gpio_reg_t* port = gpio_port(gpio_num);
  uint8_t pin = gpio_pin(gpio_num);

  port->OUTSET = (1u << pin);
}

// Inputs: 
//  gpio_num - gpio number 0-31 OR (32 + gpio number)
void gpio_clear(uint8_t gpio_num) {
  // Implement me
  // This function should make the pin low
  // It can assume that the pin has already been configured
  volatile gpio_reg_t* port = gpio_port(gpio_num);
  uint8_t pin = gpio_pin(gpio_num);

  port->OUTCLR = (1u << pin);
}

// Inputs: 
//  gpio_num - gpio number 0-31 OR (32 + gpio number)
// Output:
//  bool - pin state (true == high)
bool gpio_read(uint8_t gpio_num) {
  // Implement me
  // This function should read the value from the pin
  // It can assume that the pin has already been configured
  volatile gpio_reg_t* port = gpio_port(gpio_num);
  uint8_t pin = gpio_pin(gpio_num);
  return (port->IN & (1u << pin)) != 0;
}

// prints out some information about the GPIO driver. Can be called from main()
void gpio_print(void) {
  // Use this function for debugging purposes
  // For example, you could print out struct field addresses
  // You don't otherwise have to write anything here
  printf("GPIO P0 base: %p\n", (void*)gpio_ports[0]);
  printf("GPIO P1 base: %p\n", (void*)gpio_ports[1]);
}

