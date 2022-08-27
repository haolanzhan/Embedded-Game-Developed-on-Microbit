/**
 * @file char.h
 * @author Leanna Nguyen
 * @version 1
 * @date 6/2020
 *
 * @brief Functions for controlling the behavior and state of the character, which is a 1x1 pixel
 */
#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// struct encompassing fields necessary for the character
struct character
{
  int row;            // x pos
  int col;            // y pos
  bool state;         // active - game is running, or inactive
  int blocks_to_jump; // how many rows left for the character to move up
} mychar;

// functions prototypes
void init_char();
void check_tilt(void *unused);
void read_button();
void update_char_y(void *_unused);
void init_char();
