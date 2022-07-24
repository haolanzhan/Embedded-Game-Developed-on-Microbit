#include <stdbool.h>
#include <stdio.h>

#include "nrf_gpio.h"
#include "app_timer.h"

#include "led_matrix.h"
#include "font.h"
#include "microbit_v2.h"
#include "nrf_delay.h"
#include "char.h"
#include "lsm303agr.h"
#include "platform.h"

//initialize timer to control how fast the character falls
APP_TIMER_DEF(char_y);
//timer that controls horizontal movement
APP_TIMER_DEF(char_x);

//initialize the character
void init_char(void)
{
  mychar.row = 2;
  mychar.col = 2;
  mychar.state = true;
  mychar.blocks_to_jump = 2;

  
  //timer to move char in the y direction
  app_timer_create(&char_y, APP_TIMER_MODE_REPEATED, update_char_y);
  app_timer_start(char_y, 17000, NULL);

  //timer to move char in the x direction
  app_timer_create(&char_x, APP_TIMER_MODE_REPEATED,check_tilt);
  app_timer_start(char_x, 13000, NULL);

  
  
}

////////////////////////////////////////////////////////////////////////////
//left-right
////////////////////////////////////////////////////////////////////////////

//callback function to check tilt angle of mircrobit
//changes the character position left vs right based on tilt
void check_tilt(void* unused){
  //only move the char if the state is true (game is still in play)

  if (mychar.state == true)
    {
      //printf("this is to show that the second call back function is being called\n\n");
      lsm303agr_measurement_t accel = lsm303agr_read_accelerometer();
      float tilt_angle = get_theta(accel);
      //printf("this is my tilt angle: %f\n", tilt_angle);
      if (tilt_angle < -20)
        { //move char left
	        if (mychar.col == 0)
            {
	            //set location to column 4 if already at 1
	            mychar.col = 4; 
	          }
	        else
            {
	            //set location to column-1 if not 
	            mychar.col = mychar.col - 1; 
	          }
        }
      if (tilt_angle > 20)
      {//move char right
	      if(mychar.col == 4)
          {
	          mychar.col = 0;
	        }
	      else
          {
	          mychar.col = mychar.col + 1;
	        }
      }
    }
}

////////////////////////////////////////////////////////////////
//up-down
////////////////////////////////////////////////////////////////



//callback for updating the character, controlled by a timer. This timer can be created anywhere, but probably in led_matrix.c
//or char.c if a new file is created. The timer can be started in char_init()
void update_char_y(void* _unused)
{
  //only move the char if the state is true
  if (mychar.state == true)
    {
      //if blocks_to_jump is 0, we are falling
      if (mychar.blocks_to_jump == 0)
	{
	  //if the char is past the 4th row, game is over
	  if (mychar.row == 4)
	    {
	      mychar.state = 0;
	      printf("Your Score is: %d\n", score);
	      app_timer_stop(char_x);
	      app_timer_stop(char_y);
	      stop_platform_timers();
	      //mychar.row = 4; //keeping the char visible for testing
	    }
	  else
	    {
	      //fall down one row
	      mychar.row++;
	    }    
	}
      //if blocks_to_jump is nonzero, we have that many blocks to jump up.
      else
	{
	  if (mychar.row > 0)
	    {
	      //go up one row unless we are at the top
	      mychar.row--;
	    }
	  //decrement blocks_to_jump
	  mychar.blocks_to_jump--;
	}
    }
}
