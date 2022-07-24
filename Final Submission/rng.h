#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <string.h>


void rng_init(void);
uint8_t get_random_2to3(void);
uint8_t get_random_0to2(void);
uint8_t get_random_0to3(void);
