/**
 * @file rng.h
 * @author Haolan Zhan
 * @version 1
 * @date 6/2020
 * 
 * @brief Driver for the RNG peripheral on the NRF chip. API functions include gettings random numbers 
 * within a set range. 
 */

#ifndef RNG_H_ 
#define RNG_H_

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

void rng_init(void);
uint8_t get_random_2to3(void); // get a random number between 2 and 3
uint8_t get_random_0to2(void); // get a random number between 0 and 2
uint8_t get_random_0to3(void); // get a randome number between 0 and 3

#endif /* end if */ 
