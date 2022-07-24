// RNG Driver

#include <nrf_delay.h>

#include <stdbool.h>
#include <stdio.h>

#include "nrf_gpio.h"
#include "app_timer.h"
#include <nrfx_rng.h>


#include "microbit_v2.h"
#include "rng.h"

//struct for rng driver
typedef struct{
  uint32_t TASKS_START;
  uint32_t TASKS_STOP;
  uint32_t _unusedA[62]; //248 bytes unused in memory
  uint32_t EVENTS_VALRDY;
  uint32_t _unusedB[63]; //252 bytes unused in memory
  uint32_t SHORTS;
  uint32_t _unusedC[64]; //256 unused bytes
  uint32_t INTENSET;
  uint32_t INTENCLR;
  uint32_t _unusedD[126];
  uint32_t CONFIG;
  uint32_t VALUE;
}rng_reg_t;

volatile rng_reg_t* MY_RNG_REGS = (rng_reg_t*)(0x4000D000);

void rng_init(void)
{
  /*
  printf("TASKS_START: %p\n", &(MY_RNG_REGS->TASKS_START));
  printf("EVENTS: %p\n", &(MY_RNG_REGS->EVENTS_VALRDY));
  printf("SHORTS: %p\n", &(MY_RNG_REGS->SHORTS));
  printf("INTENSET: %p\n", &(MY_RNG_REGS->INTENSET));
  printf("VALUE: %p\n", &(MY_RNG_REGS->VALUE));
  */

  //disable interups, disable shorts, and enable bias for even distribution
  MY_RNG_REGS->CONFIG = 1;
  MY_RNG_REGS->INTENCLR = 1;
  MY_RNG_REGS->SHORTS = 0;
}

uint8_t get_random_2to3(void)
{
  //start rng generation and read a value between 0 and 1
  MY_RNG_REGS->TASKS_START = 1;
  nrf_delay_ms(2);
  uint8_t random_number = (MY_RNG_REGS->VALUE) & 1;
  MY_RNG_REGS->TASKS_STOP = 1;

  //only return 2 or 3
  return (random_number + 2); 
}

uint8_t get_random_0to2(void)
{
  //start rng generation and read a value between 0 and 3
  MY_RNG_REGS->TASKS_START = 1;
  nrf_delay_ms(2);
  uint8_t random_number = (MY_RNG_REGS->VALUE) & 3;
  MY_RNG_REGS->TASKS_STOP = 1;

  //only return 0-2
  if (random_number == 3)
    {
      random_number = 1;
    }

  return random_number;
}

uint8_t get_random_0to3(void)
{
  //start rng generation and read a value between 0 and 3
  MY_RNG_REGS->TASKS_START = 1;
  nrf_delay_ms(2);
  uint8_t random_number = (MY_RNG_REGS->VALUE) & 3;
  MY_RNG_REGS->TASKS_STOP = 1;

  //only return 2 or 3
  return random_number; 
}
