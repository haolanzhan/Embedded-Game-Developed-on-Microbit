/**
 * @file platform.c
 * @author Haolan Zhan
 * @version 1
 * @date 6/2020
 * 
 * @brief Functions that define the state and behavior of the falling platforms on the 5x5 LED matrix
 */

#include <nrf_delay.h>

#include <stdbool.h>
#include <stdio.h>

#include "nrf_gpio.h"
#include "app_timer.h"

#include "led_matrix.h"
#include "font.h"
#include "microbit_v2.h"
#include "rng.h"
#include "platform.h"

/*****************************************************
 * @note Timer definitions using the NRF timer library
 ****************************************************/
//timers that control platforms
APP_TIMER_DEF(timer_falling_platforms);
APP_TIMER_DEF(timer_activate_platforms);

//timer to display screen - technically not platform related, but related to the display of the entire 5x5 LED matrix. 
//Moved here to keep all timer definitions in the same place 
APP_TIMER_DEF(display_screen);

/*****************************************************
 * @note API functions
 ****************************************************/

/**
 * @brief Initialize the platform vector
 */
void platform_init(void)
{
  //Set the state of the first platform as true, the rest to false
  for (int i = 0; i<NUMBER_OF_PLATFORMS; i++)
  {
    if (i == 1)
	  {
	    my_platform_vector[i].state = true;
	  }
    else
	  {
	    my_platform_vector[i].state = false;
	  }
      
    my_platform_vector[i].row = 0; //every platform starts at row 0 (top row)
    randomize_platform(i); //randomize the length and x-offset position of each platform
  }

  //initialize score
  score = 0;

  //LED MATRIX INIT MUST BE CALLED FIRST!
  //technically not related to platforms, but the callback for displaying the entire 5x5 Matrix
  app_timer_create(&display_screen, APP_TIMER_MODE_REPEATED,part4_cb); //params: timer variable address, repeated mode, callback 
  app_timer_start(display_screen, 16, NULL); //params: timer variable, timout ticks (lower number means a faster timer)
  
  //start the timers to cause the platforms to fall - rate at which platform is falling/moving down by one row
  app_timer_create(&timer_falling_platforms, APP_TIMER_MODE_REPEATED, next_row); 
  app_timer_start(timer_falling_platforms, 26000, NULL);

  //timer to activate platforms - rate at which inactive platforms are activated and begin falling down
  app_timer_create(&timer_activate_platforms, APP_TIMER_MODE_REPEATED, activate_platform);
  app_timer_start(timer_activate_platforms, 65000, NULL);
}

/**
 * @brief Stop the platform related timers when the game is over
 * 
 */
void stop_platform_timers(void)
{
  app_timer_stop(timer_activate_platforms);
  app_timer_stop(timer_falling_platforms);

  //stop displaying platforms by setting all platform states to false
  for(int i = 0; i<total_platforms; i++)
  {
    my_platform_vector[i].state = false;
  }
}

/**
 * @brief set the length and location of a platform using the RNG driver
 * 
 * @param i which platform struct in the vector to set
 */
void randomize_platform(int i)
{
  //choose between a platform that is 2 or 3 leds wide 
  my_platform_vector[i].size = get_random_2to3();

  //set the offset based on the platform size
  if (my_platform_vector[i].size == 2)
  {
    my_platform_vector[i].offset = get_random_0to3();
  }
  else
  {
    my_platform_vector[i].offset = get_random_0to2(); //becase the platform is bigger, it can no longer have an offset of 3
  }
}

/**
 * @brief callback function to activate first unused platforms regularly
 * @param _unused 
 * @note this function will be called when the corresponding timer fires 
 */
void activate_platform(void* _unused)
{
  //loop through each platform, activating the first one that is inactive
  for (int i = 0; i<total_platforms; i++)
  {
    if (my_platform_vector[i].state == false)
	    {
	      my_platform_vector[i].state = true;
	      break;
	    }
  }
}

/**
 * @brief Callback function to cause the platforms to fall
 * @param _unused 
 * @note 
 * called when the corresponding timer fires
 * 
 * Row numbering begins with 0 at the top, to 4 on the bottom
 */
void next_row(void* _unused)
{
  //loop through all platforms
  for (int i = 0; i<total_platforms; i++)
  {
    //make the platform fall if the state is on
    if (my_platform_vector[i].state == true)
	  {
	    if (my_platform_vector[i].row == 4)
	    {
		    //if the platform has reached the bottom, turn the state off, send it back to the top to be released again,
		    //and randomize the size and location
		    my_platform_vector[i].row = 0;
		    my_platform_vector[i].state = false;
		    randomize_platform(i);
		      
        //update score for each platform that falls below the bottom 
		    score += 100; 
	    }
	    else
	    {
		    my_platform_vector[i].row++; //increase the row number (resulting in the platform falling one row)
	    }
	  }
  }
}
