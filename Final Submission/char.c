/**
 * @file char.c
 * @author Leanna Nguyen
 * @version 1
 * @date 6/2020
 *
 * @brief Functions for controlling the behavior and state of the character, which is a 1x1 pixel
 */
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

// timer to control how fast the character falls
APP_TIMER_DEF(char_y);
// timer that controls horizontal movement
APP_TIMER_DEF(char_x);

/**
 * @brief initialize the character
 *
 * @note
 * Character movement in both axis will be at a constant rate. However, movement is defined as left, right, or zero,
 * therefore where the character moves is dependent on the state of other variables
 */
void init_char(void)
{
	// initialize character to center of the LED matrix
	mychar.row = 2;
	mychar.col = 2;
	mychar.state = true;
	mychar.blocks_to_jump = 2; // initialize the character with an upwards jump by two rows

	// create and start timer to move char in the y direction
	app_timer_create(&char_y, APP_TIMER_MODE_REPEATED, update_char_y);
	app_timer_start(char_y, 17000, NULL);

	// create and start timer to move char in the x direction
	app_timer_create(&char_x, APP_TIMER_MODE_REPEATED, check_tilt);
	app_timer_start(char_x, 13000, NULL);
}

/**
 * @brief Callback function to check the tilt of the microbit,
 * and update the x-pos of the character (i.e. left-right movement)
 *
 * @note called when the char_x timer fires
 * leftmost column is index 0, right most column is index 4
 */
void check_tilt(void *unused)
{
	// only move the char if the state is true (game is still in play)
	if (mychar.state == true)
	{
		// printf("this is to show that the second call back function is being called\n\n");

		// get accelerometer data
		lsm303agr_measurement_t accel = lsm303agr_read_accelerometer();

		// get the tilt angle
		float tilt_angle = get_theta(accel);
		// printf("this is my tilt angle: %f\n", tilt_angle);

		if (tilt_angle < -20)
		{
			// move char left
			if (mychar.col == 0)
			{
				// set location to column 4 if already at 0, thus looping around to the other side
				mychar.col = 4;
			}
			else
			{
				// set location to column-1 if not
				mychar.col = mychar.col - 1;
			}
		}

		if (tilt_angle > 20)
		{
			// move char right
			if (mychar.col == 4)
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

/**
 * @brief callback for updating the up-down movement of the character
 *
 * @note function is called when the corresponding timer fires
 *
 * The top row is index 0, the bottom row is index 4
 */
void update_char_y(void *_unused)
{
	// only move the char if the state is true
	if (mychar.state == true)
	{
		// if blocks_to_jump is 0, we are falling
		if (mychar.blocks_to_jump == 0)
		{
			// if the char is past the 4th row, game is over. Stop timers, and display score
			if (mychar.row == 4)
			{
				mychar.state = 0;
				printf("Your Score is: %d\n", score);
				app_timer_stop(char_x);
				app_timer_stop(char_y);
				stop_platform_timers();
				// mychar.row = 4; //keeping the char visible for testing
			}
			else
			{
				// fall down one row
				mychar.row++;
			}
		}
		// if blocks_to_jump is nonzero, we have that many blocks to jump up. Jump up one row now, decrement the jump counter,
		// and wait for the next instance of the timer firing to continue the jump
		else
		{
			if (mychar.row > 0) // if we are at the top, remain at the top row
			{
				// go up one row unless we are at the top
				mychar.row--;
			}
			// decrement blocks_to_jump
			mychar.blocks_to_jump--;
		}
	}
} 
