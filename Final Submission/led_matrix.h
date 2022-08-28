/**
 * @file led_matrix.h
 * @author Haolan Zhan | Leanna Nguyen
 * @version 1
 * @date 6/2020
 *
 * @brief Functions for controlling the display on the LED matrix
 */
#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

//Initialize the LED matrix display
void led_matrix_init(void);
//main callback functions to refresh the screen
void part4_cb(void* unused);
//callback for string character update from lab
void update_string(void* unused);
//callback to display character from lab
void display_ascii(void* unused);
//callback to check if button pressed
void check_if_buttons_pressed(void* unused);
//display a char on the LED
void map_char(char c);
//set LED states of the row
void set_states(int led_states_row, uint8_t row);
//display a string on the LED
void iterate_string(const char* string);

/**
 * @note Helper functions 
 */

//clear the led states
void clear_led_states(void);
//set the led states with the location of the platforms
void update_state_with_platforms(void);
//collision detection between character and platforms
void detect_collision(void);
//set ledstaets with location of character
void update_led_states_with_char_pos(void);
//cycle through the duty cycle
void increment_pwm_index(int);

//legacy
//static void part6_cb(void* unused);
//static void display_character(void* unused);
