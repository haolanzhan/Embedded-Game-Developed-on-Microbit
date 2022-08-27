/**
 * @file platform.h
 * @author Haolan Zhan
 * @version 1
 * @date 6/2020
 * 
 * @brief Functions that define the state and behavior of the falling platforms on the 5x5 LED matrix
 */

#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define NUMBER_OF_PLATFORMS 3 //since we only have 5 rows to work with on the LED matrix, there is no point in having
                              //more than 3 platforms, since a max of 2 will be visible at a time while the third one
                              //will be inactive, waiting for its turn to fall. More than 2 platforms being visible 
                              //at a time will look too cramped, and will not result in good gameplay 

//keep track of score
int score;

//struct for platform (global variable)
struct platform_s{
  bool state; //active state means that the platform is visible on the screen and currently falling
  uint8_t size; //horizontal length of platform in pixels (platform is always one pixel vertically)
  uint8_t row; //which row the platform occupies on the LED matrix
  uint8_t offset; //the offset from the left edge of the 5x5 LED matrix to define the x-position of the platform 
} my_platform_vector[NUMBER_OF_PLATFORMS]; //initilize a vector of these structs

//API function, view the dozygen comments in the .c file for information
void platform_init(void);
void next_row(void*); 
void activate_platform(void*);
void randomize_platform(int i);
void stop_platform_timers(void);
