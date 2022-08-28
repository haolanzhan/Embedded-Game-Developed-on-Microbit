/**
 * @file led_matrix.c
 * @author Haolan Zhan | Leanna Nguyen
 * @version 1
 * @date 6/2020
 *
 * @brief Functions for controlling the display on the LED matrix
 * @note
 * For this board, the 5x5 LED matrix is wired so that each column is connected electrically.
 * It is impossible to have two LEDS from different rows to be active at the same time unless
 * they are in the same column. Because of this, the procedure for displaying a graphic on the 5x5 matrix
 * is by the following procedure:
 *      turn off all rows
 *      set the desired columns active (nothing will be displayed because the rows are off)
 *      turn on a single row (the columns that are already active will have the LEDs turn on)
 *      turn off all rows
 *      reset the desired columns active
 *      turn on the next row (and the new active columns will have the LEDs on)
 *
 * Cyling through each row like this at a high enough frequency will give the illusion of a continous image
 * on the 5x5 array
 */

#include <stdbool.h>
#include <stdio.h>

#include "nrf_gpio.h"
#include "app_timer.h"

#include "led_matrix.h"
#include "font.h"
#include "microbit_v2.h"
#include "nrf_delay.h"
#include "platform.h"
#include "char.h"
#include "lsm303agr.h"

/******************************************************************
 * @note global variables for controlling LED brightness with PWM
 *****************************************************************/
uint8_t pwm_index; //index into the third dimension of the led_states variable below
uint8_t duty_cycle; //equivalent to toggle_point/countertop in terms of percentage
#define countertop 4 //defines the limit of PWM cycles available 
uint8_t toggle_point; //sets the brightness of the platform, should be between 1 and countertop
//the character has a maximum brightness, thus it's own toggle_point will be equal to the countertop, and thus we don't need a variable for it

/******************************************************************
 * @note global variables related to LED states
 *****************************************************************/
// 3D matrix that holds states of LEDs, and pwm duty cycles. The first dimension is the row index, the second
// dimension is the column index, and the third dimension will be how PWM will be implemented. There are 5 rows and
// 5 columns, and 4 possible PWD settings: 0 through 4, where 4 represents maximum brightness, and 0 represents minimum
// brightness. How this will work is that depending on what number the PWM dimension is, that represents the percentage of
// time which that LED is on if that specific LED state is active. If LED [2,1] is inactive, the PWM number doesnt matter. If
// LED [2,1] is active, and the PWM dimension is 3, that means 75% of the time this LED is on, but the frequency is too fast
// therefore the LED looks continuously on, but dimmer than if the PWM dimension is 4 (100% on)
// PWM is for making the platform and the character different brightness
bool led_states[5][5][countertop] = {false};
bool led_states_string[5][5] = {false};                                // separate state for displaying strings, no PWM is necessary
int curr_row = 0;                                                      // default current row = 0, represents which row the LED matrix should currently display
int curr_row_string = 0;                                               // default current row = 0, represents which row the LED matrix should currently display
uint32_t rows[5] = {LED_ROW1, LED_ROW2, LED_ROW3, LED_ROW4, LED_ROW5}; // pin definitions for LED rows
uint32_t cols[5] = {LED_COL1, LED_COL2, LED_COL3, LED_COL4, LED_COL5}; // pin definitions for LED columns

/******************************************************************
 * @note Necessary timers
 *****************************************************************/
APP_TIMER_DEF(display_string); // timer that displays the gameplay strings
APP_TIMER_DEF(update_ascii);   // timer that updates to the next ascii character
APP_TIMER_DEF(button_check);   // timer that runs to check if a button is pressed

/******************************************************************
 * @note Global variables for string display
 *****************************************************************/
uint8_t curr_char;
const char *mystring;
bool string_done = false;

/******************************************************************
 * @note PWM function
 *****************************************************************/

/**
 * @brief Cycle through the duty cycles
 *
 * @param next_row - next row index
 * @note - The sequence of display is the following:
 * Display row 0, all columns, duty cycles 0 through countertop-1
 * Display row 1, all columns, duty cycles 0 through countertop-1
 * ...
 * Display row 4, all columns, duty cycles 0 through countertop-1
 *
 * repeat
 */
void increment_pwm_index(int next_row)
{
  // cycle through the pwm duty cycles
  if (pwm_index == (countertop - 1))
  {
    // if we reach the end of one duty cycle, print the next row
    pwm_index = 0;
    curr_row = next_row;
  }
  else
  {
    // continue going through the duty cycle
    pwm_index++;
  }
}

/******************************************************************
 * @note Platform related functions
 *****************************************************************/

/**
 * @brief update led_states with platform locations
 */
void update_state_with_platforms(void)
{
  uint8_t row; //which row the platform is one
  uint8_t start_index; //column index where platform begins
  uint8_t end_index; //column index where platform ends

  //Loop through each platform struct
  for (int i = 0; i < NUMBER_OF_PLATFORMS; i++)
  {
    // check if the platform is actually being displayed
    if (my_platform_vector[i].state == true)
    {
      // get location of the platform row, start column, and end column
      row = my_platform_vector[i].row;
      start_index = my_platform_vector[i].offset;
      end_index = my_platform_vector[i].size + start_index - 1;

      //loop through each column index
      //set the specified coordinates in led_states to true
      for (int j = 0; j < 5; j++)
      {
        //bounds of the current platform
        if ((j >= start_index) && (j <= end_index))
        {
          // set the duty cycle depending on the countertop and togglepoint (togglepoint determines brightness)
          // for example, if the toggle_point is 1, the third dimention of LED [x,y] would be ON OFF OFF OFF
          for (int k = 0; k < countertop; k++)
          {
            if (k < toggle_point)
            {
              led_states[row][j][k] = true;
            }
            else
            {
              led_states[row][j][k] = false;
            }
          }
        }
      }
    }
  }
}

/******************************************************************
 * @note Character control related functions
 *****************************************************************/

/**
 * @brief Collision detection between platform and character
 * 
 * @note when this function is called, it is assumed that LED_STATES is 
 * already populated based on the platform location. Thus, if the char is at 
 * position (mychar.row, mychar.col), and this same position in the led_states
 * is on, than that means that the character is where the platform is, and collision
 * must be detected
 */
void detect_colision()
{
  // the first duty cycle should always be true if the led is suppose to be on, we detect the collision based on that
  if (led_states[mychar.row][mychar.col][0] == true)
  {
    // only if the character is falling, will colliding cause the character to jump up 2 more blocks
    // the character will not collide on the way up through a platform, just like doodle jump
    if (mychar.blocks_to_jump == 0)
    {
      mychar.blocks_to_jump = 2; //give the character two rows to jump up, will be handled in the timer callback for the character
    }
  }
}

/**
 * @brief Update the LED_STATES with the position of the character
 * 
 * @note 
 * this function is called everytime the screen is drawn, along with update_platforms() and clear_screen()
 * the screen should clear first, and then the led_states should be updated with the platform locations, 
 * and then we call this function (therefore our collision detection should function properly)
 * 
 * These functions should be called in the main callback function that draws the screen
 * 
 * Because the platform states are set first under this architecture, we can safetly detect collision, and then
 * override the led_states matrix with the character state, and set every value in the third dimension to true 
 * in order to give the character maximum brightness (and thus be brighter than the platform, if the two overlap)
 */
void update_led_states_with_char_pos(void)
{
  // if the game is still going
  if (mychar.state == true)
  {
    // check for collisions
    detect_colision();
    // override led_states each time to give the character maximum brightness (100% duty cycly)
    // loop through the 3rd dimension of led_states and set the state of the char LED to true 
    for (int k = 0; k < countertop; k++)
    {
      led_states[mychar.row][mychar.col][k] = true;
    }
  }
}

/******************************************************************
 * @note Main callback function to display the LED Matrix 
 *****************************************************************/

/**
 * @brief main callback function to display the LED screen
 * 
 * @note This will be called upon the firing of the display_screen timer, which is 
 * defined and started in platform.c 
 * 
 * The frequency that this function is called must be fast enough so that it is called 20 times before the eye
 * can process a separate image ... the display must cycle through each of the 4 duty cycles for a single row, and
 * cycle through all 5 rows, in order for the eye to see a continuous 5x5 image 
 */
void part4_cb(void *unused)
{
  // clear the states, then update the led_sates with platform and character positions
  clear_led_states();
  update_state_with_platforms();
  update_led_states_with_char_pos();

  /* -- Below is the code to draw -- */ 

  // deactivate the current row
  uint32_t row = rows[curr_row];
  nrf_gpio_pin_write(row, 0);

  // temporary variable to access the next_row without changing rows
  // we only change to the next row after all duty cycles for the current row is displayed 
  int next_row;
  if (curr_row < 4)
  {
    next_row = curr_row + 1; //get the next row
  }
  else
  {
    next_row = 0; //next row loops back to the top
  }

  uint32_t col; //variable to store colummn index

  // change column pin states:
  for (int i = 0; i < 5; i = i + 1)
  {
    col = cols[i]; // get the LED_COL pin definition from the col array
    if (led_states[next_row][i][pwm_index] == true)
    {
      // get element from LED states array for given row and duty cycle
      // if it's true, we want the light to stay on
      nrf_gpio_pin_write(col, 0);
    }
    else
    {
      // if the column is false, set it to 1 to turn it off
      nrf_gpio_pin_write(col, 1);
    }
  }

  // enable next row
  row = rows[next_row]; // get the next LED_ROW pin definition from LED_ROW array
  nrf_gpio_pin_write(row, 1); // enable that row (make high)

  // update pwm and row (if neccessary)
  increment_pwm_index(next_row);
}

/******************************************************************
 * @note Functions for displaying strings onto Matrix. Use it's own
 * timer and callback fuction similar to part4_cb(), but without the
 * complicated duty cycle feature
 *****************************************************************/

/**
 * @brief Update the LED_STATES with the LEDs needed to make out a character
 * 
 * @note this callback function is associated with its corresponding timer. 
 * The frequency that this timer fires is the rate at which the LED_STATES is 
 * updated with the next char, and thus the rate which the characters scroll
 * on the LED matrix, being displayed one at a time. 
 */
void update_string(void *unused)
{ 
  // this cb updates the next letter
  map_char(mystring[curr_char]); //set the LED states to the character we want to display
  if (curr_char < strlen(mystring))
  {
    curr_char = curr_char + 1; //prepare to display the next char
  }

  else
  {
    //the string has finished displaying, thus we stop both timers
    app_timer_stop(display_string);
    app_timer_stop(update_ascii); 
  }
}

/**
 * @brief Display the character on the LED matrix using the updated states from update_string()
 * 
 * @note everytime the corresponding timer fires, one row is displayed on the LED matrix, and 
 * we prepare to display the next row. Cycling through the rows fast enough creates a solid image to 
 * the human eye.  
 * 
 * whatever current row is, it represents the last row that was displayed. Thus, each call of this 
 * function will display the next row. 
 */
void display_ascii(void *unused)
{
  uint32_t row = rows[curr_row_string]; // get current row
  nrf_gpio_pin_write(row, 0); // turn off the current row 

  // increment curr_row_string for so that we can display the next row in this callback function
  // after this, curr_row_string now holds the next row
  if (curr_row_string < 4)
  {
    curr_row_string = curr_row_string + 1; //get the next row below the current one
  }
  else
  {
    curr_row_string = 0; //loop back to the first row if we have reached the bottom
  }

  // change column pin states by looping through each column for the current row
  for (int i = 0; i < 5; i = i + 1)
  {
    uint32_t col = cols[i]; // get the LED_COL pin definitoin from the col array
    if (led_states_string[curr_row_string][i] == true)
    {
      // get element from LED states array for given row
      // if it's true, we want the light to stay on
      nrf_gpio_pin_write(col, 0);
    }
    else
    {
      // if the column is false, set it to 1 to turn off the column 
      nrf_gpio_pin_write(col, 1);
    }
  }

  // enable next row
  row = rows[curr_row_string]; // get the next LED_ROW from LED_ROW array
  nrf_gpio_pin_write(row, 1);  // enable that row (make high)
}

/**
 * @brief set the states of the led_matrix to prepare to print a string 
 * 
 * @param c - the character we want to display
 */
void map_char(char c)
{
  // get char ascii value
  int c_num = c;
  // get hex values for each LED row from the font array
  // font[i] represents a specific character
  // font[i][j] represents the state of each column of the jth row to display that character
  uint8_t row1 = font[c_num][0];
  uint8_t row2 = font[c_num][1];
  uint8_t row3 = font[c_num][2];
  uint8_t row4 = font[c_num][3];
  uint8_t row5 = font[c_num][4];

  set_states(0, row1);
  set_states(1, row2);
  set_states(2, row3);
  set_states(3, row4);
  set_states(4, row5);
}

/**
 * @brief Set a single row of the LED_STATES 
 * 
 * @param led_states_row -  which row to change the state for in led_states_string
 * @param row - holds the actual desired state of each column in that row
 */
void set_states(int led_states_row, uint8_t row)
{
  // we need to isolate the state of that row with a shift and &mask
  // column 0 represents the lsb of the input row, and so on 
  led_states_string[led_states_row][0] = (row)&1;
  led_states_string[led_states_row][1] = (row >> 1) & 1;
  led_states_string[led_states_row][2] = (row >> 2) & 1;
  led_states_string[led_states_row][3] = (row >> 3) & 1;
  led_states_string[led_states_row][4] = (row >> 4) & 1;
}

/**
 * @brief display the given string by starting the necessary timers
 * 
 * @param string - takes in a pointer to the first character in the string, i.e. an char array 
 */
void iterate_string(const char *string)
{
  mystring = string;
  curr_char = 0;
  app_timer_start(update_ascii, 32768 / 4, NULL); //start the timer to update the led_states based on the next char
  /*while (!string_done){
    //printf("still printing");
  }
  printf('going to stop the display string timer\n');
  //app_timer_stop(display_string);
  //string_done = true;
  app_timer_stop(update_ascii);
  //app_timer_stop(display_string);
  //app_timer_start(button_check,100,NULL);*/
}

/******************************************************************
 * @note Game play functions 
 *****************************************************************/

/**
 * @brief Check to see if either buttons on the micro:bit have been pressed
 * 
 * @note this callback function is associated with a timer that so that 
 * we can periodically check either buttons
 * 
 * Upon the button press, we start the game (transitioning from the initial screen that was
 * displaying a string, to the actual play state)
 */
void check_if_buttons_pressed(void *unused)
{
  // printf("button check callback\n");

  // check either buttons
  if ((nrf_gpio_pin_read(BTN_A) == 0) | (nrf_gpio_pin_read(BTN_B) == 0))
  {
    //stop timers that are printing the current string, and initialize the character and the platform
    app_timer_stop(button_check); //stop its own timer checking for buttons too
    app_timer_stop(update_ascii); 
    app_timer_stop(display_string);
    platform_init(); //this begins the timer that is associated with part4_cb(), which is the main callback of the game 
    init_char();
  }
}

/**
 * @brief Initialize and start timers in order for the game to begin upon power up - loads the intro state 
 * where a string is being displayed on the LED matrix
 * 
 */
void led_matrix_init(void)
{
  // initialize row pins
  nrf_gpio_pin_dir_set(LED_ROW1, NRF_GPIO_PIN_DIR_OUTPUT);
  nrf_gpio_pin_dir_set(LED_ROW2, NRF_GPIO_PIN_DIR_OUTPUT);
  nrf_gpio_pin_dir_set(LED_ROW3, NRF_GPIO_PIN_DIR_OUTPUT);
  nrf_gpio_pin_dir_set(LED_ROW4, NRF_GPIO_PIN_DIR_OUTPUT);
  nrf_gpio_pin_dir_set(LED_ROW5, NRF_GPIO_PIN_DIR_OUTPUT);
  // initialize col pins
  nrf_gpio_pin_dir_set(LED_COL1, NRF_GPIO_PIN_DIR_OUTPUT);
  nrf_gpio_pin_dir_set(LED_COL2, NRF_GPIO_PIN_DIR_OUTPUT);
  nrf_gpio_pin_dir_set(LED_COL3, NRF_GPIO_PIN_DIR_OUTPUT);
  nrf_gpio_pin_dir_set(LED_COL4, NRF_GPIO_PIN_DIR_OUTPUT);
  nrf_gpio_pin_dir_set(LED_COL5, NRF_GPIO_PIN_DIR_OUTPUT);
  // set default values for pins
  nrf_gpio_pin_clear(LED_COL1);
  nrf_gpio_pin_clear(LED_COL2);
  nrf_gpio_pin_clear(LED_COL3);
  nrf_gpio_pin_clear(LED_COL4);
  nrf_gpio_pin_clear(LED_COL5);
  nrf_gpio_pin_clear(LED_ROW1);
  nrf_gpio_pin_clear(LED_ROW2);
  nrf_gpio_pin_clear(LED_ROW3);
  nrf_gpio_pin_clear(LED_ROW4);
  nrf_gpio_pin_clear(LED_ROW5);

  // initialize pwm global var
  pwm_index = 0;
  duty_cycle = 25; // a percentage 
  toggle_point = (duty_cycle * countertop) / 100; // the duty cycle as a fraction of the countertop

  // initialize buttons A and B for game play
  nrf_gpio_pin_dir_set(BTN_A, NRF_GPIO_PIN_DIR_INPUT); // config button A P0.14
  nrf_gpio_pin_dir_set(BTN_B, NRF_GPIO_PIN_DIR_INPUT); // config button B P0.23

  // initialize the timer peripheral, and the timers relayed to displaying the intro string as well as the 
  // timer checking for a button press
  app_timer_init(); 
  app_timer_create(&display_string, APP_TIMER_MODE_REPEATED, display_ascii);
  app_timer_create(&update_ascii, APP_TIMER_MODE_REPEATED, update_string);
  app_timer_create(&button_check, APP_TIMER_MODE_REPEATED, check_if_buttons_pressed);

  //start the timer to print out the initial string 
  app_timer_start(display_string, 65, NULL);
  //print the string on the LED matrix (this function will start the update_ascii timer)
  iterate_string("Play Doodle Jump! Press a button to start.");
  //start the timer to check for button press for the game to start
  app_timer_start(button_check, 100, NULL);
  // printf("starting button_check timer\n");
}

/**
 * @brief Functino that clears the LED states to reset the screen for the main callback function 
 * 
 */
void clear_led_states(void)
{
  for (int i = 0; i < 5; i++)
  {
    for (int j = 0; j < 5; j++)
    {
      for (int k = 0; k < countertop; k++)
      {
        led_states[i][j][k] = false;
      }
    }
  }
}