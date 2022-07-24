// Controller for the platforms

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

//timers that control platforms
APP_TIMER_DEF(timer_falling_platforms);
APP_TIMER_DEF(timer_activate_platforms);

//timer to display screen
APP_TIMER_DEF(display_screen);

//total_platform number
#define total_platforms 3

void platform_init(void)
{
  //initialize platform vector
  for (int i = 0; i<total_platforms; i++)
    {
      if (i == 1)
	{
	  my_platform_vector[i].state = true;
	}
      else
	{
	  my_platform_vector[i].state = false;
	}
      my_platform_vector[i].row = 0;
      randomize_platform(i);
    }

  //initialize score
  score = 0;

  //LED MATRIX INIT MUST BE CALLED FIRST!
  //timer to cause the platforms to fall
  app_timer_create(&display_screen, APP_TIMER_MODE_REPEATED,part4_cb);
  app_timer_start(display_screen, 16, NULL); //moved it here
  app_timer_create(&timer_falling_platforms, APP_TIMER_MODE_REPEATED, next_row);
  app_timer_start(timer_falling_platforms, 26000, NULL);

  //timer to activate platforms
  app_timer_create(&timer_activate_platforms, APP_TIMER_MODE_REPEATED, activate_platform);
  app_timer_start(timer_activate_platforms, 65000, NULL);
}

//stop the platform timers
void stop_platform_timers(void)
{
  app_timer_stop(timer_activate_platforms);
  app_timer_stop(timer_falling_platforms);

  //stop displaying platforms
  for(int i = 0; i<total_platforms; i++)
    {
      my_platform_vector[i].state = false;
    }
}

//randomize the rows and offset of each falling platform
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
      my_platform_vector[i].offset = get_random_0to2();
    }
}

//callback function to activate first unused platforms regularly
void activate_platform(void* _unused)
{
  for (int i = 0; i<total_platforms; i++)
    {
      if (my_platform_vector[i].state == false)
	{
	  my_platform_vector[i].state = true;
	  break;
	}
    }
    
}

//call back function to cause the platforms to fall
void next_row(void* _unused)
{
  for (int i = 0; i<total_platforms; i++)
    {
      //make the platform fall if the state is on
      if (my_platform_vector[i].state == true)
	{
	    if (my_platform_vector[i].row == 4)
	      {
		//if the platform has reached the bottom, turn it off, sent it back to the top to be released again,
		//and randomize the size and location
		my_platform_vector[i].row = 0;
		my_platform_vector[i].state = false;
		randomize_platform(i);
		//update score
		score += 100; 
	      }
	    else
	      {
		my_platform_vector[i].row++;
	      }
	}
    }
}
