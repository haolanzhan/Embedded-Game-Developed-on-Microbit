/**
 * @file rng.c
 * @author Haolan Zhan
 * @version 1
 * @date 6/2020
 * 
 * @brief Driver for the RNG peripheral on the NRF chip. API functions include gettings random numbers 
 * within a set range. 
 */

#include <nrf_delay.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include "nrf_gpio.h"
#include "app_timer.h"
#include <nrfx_rng.h>
#include "microbit_v2.h"
#include "rng.h"

/************************************
 * @note structs and global variables 
 ***********************************/

//struct for rng driver based on available registers 
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

volatile rng_reg_t* MY_RNG_REGS = (rng_reg_t*)(0x4000D000); // use base adress for the RNG register

/************************************
 * @note Driver functions 
 ***********************************/

/**
 * @brief Initialize the RNG peripheral by setting the appropriate settings in the config registers
 */
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

/**
 * @brief get a random number between 2 and 3
 */
uint8_t get_random_2to3(void)
{
  //start rng generation and read a value between 0 and 1
  MY_RNG_REGS->TASKS_START = 1;
  nrf_delay_ms(2);
  uint8_t random_number = (MY_RNG_REGS->VALUE) & 1; //only use LSB of the random valie
  MY_RNG_REGS->TASKS_STOP = 1;

  //only return 2 or 3
  return (random_number + 2); 
}

/**
 * @brief get a random number between 0 and 2
 * @note Hack: getting a random number with a range of 3 is awkward, need to hardcode to return 1 if the
 * random value is 3. This results in 1 being more likely than 0 or 2, but since that is the median value, 
 * this behavior is acceptable
 */
uint8_t get_random_0to2(void)
{
  //start rng generation and read a value between 0 and 3
  MY_RNG_REGS->TASKS_START = 1;
  nrf_delay_ms(2);
  uint8_t random_number = (MY_RNG_REGS->VALUE) & 3; //only use the least significant 2 bits 
  MY_RNG_REGS->TASKS_STOP = 1;

  //only return 0-2
  if (random_number == 3)
    {
      random_number = 1;
    }

  return random_number;
}

/**
 * @brief get a random number between 0 and 3
 */
uint8_t get_random_0to3(void)
{
  //start rng generation and read a value between 0 and 3
  MY_RNG_REGS->TASKS_START = 1;
  nrf_delay_ms(2);
  uint8_t random_number = (MY_RNG_REGS->VALUE) & 3; //only use the least significant 2 bits
  MY_RNG_REGS->TASKS_STOP = 1;

  //only return 0-3
  return random_number; 
}
