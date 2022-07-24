#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <string.h>


struct character{
  int row;
  int col;
  bool state;
  int blocks_to_jump;
} mychar;

void init_char();
void check_tilt(void* unused);
void read_button();
void update_char_y(void* _unused);
void init_char();
