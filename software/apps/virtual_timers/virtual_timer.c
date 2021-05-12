// Virtual timer implementation

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "nrf.h"

#include "virtual_timer.h"
#include "virtual_timer_linked_list.h"

// This is the interrupt handler that fires on a compare event
void TIMER4_IRQHandler(void) {
  // This should always be the first line of the interrupt handler!
  // It clears the event so that it doesn't happen again
  NRF_TIMER4->EVENTS_COMPARE[2] = 0;

  // Place your interrupt handler code here
  printf("Timer Fired!\n");
  node_t* timer = list_remove_first(); //???
  timer->cb(); 
  uint32_t rep = timer->repeated; //get value to see if repeated
  if (rep != 0){
    // if repeated update expiration time in linked list and CC
    uint32_t timer_val = read_timer(); // gives current time
    timer->timer_value = timer_val + rep;
    list_insert_sorted(timer);
  }
  else{
    free(timer);
  }
  
  cc_update();
}

// Read the current value of the timer counter
uint32_t read_timer(void) {

  // Should return the value of the internal counter for TIMER4

  //when a value of 1 is written to TASKS_CAPTURE[n] register,
  //the value of the timer is copied to the corresponding CC[n] register

  NRF_TIMER4->TASKS_CAPTURE[1] = 1;
  return NRF_TIMER4->CC[1];
}

// Initialize TIMER4 as a free running timer
// 1) Set to be a 32 bit timer
// 2) Set to count at 1MHz
// 3) Enable the timer peripheral interrupt (look carefully at the INTENSET register!)
//    Skip this for the first part of the lab.
// 4) Clear the timer
// 5) Start the timer
void virtual_timer_init(void) {
  // Place your timer initialization code here
  NRF_TIMER4->BITMODE = 0x3; //set to a 32 bit timer
  NRF_TIMER4->PRESCALER = 0x4;  //want 1MHz so our prescaler will be 4
  NRF_TIMER4->TASKS_CLEAR = 1;  //clear the timer
  NRF_TIMER4->TASKS_START = 1;  //start the timer

  //enable the interrupt using INTENSET
  //it's enabled when bits 16:21 = 1
  NRF_TIMER4->INTENSET = 1 << 18; //enables interrupt for CC[2]
  NVIC_EnableIRQ(TIMER4_IRQn);
}

// Start a timer. This function is called for both one-shot and repeated timers
// To start a timer:
// 1) Create a linked list node (This requires `malloc()`. Don't forget to free later)
// 2) Setup the linked list node with the correct information
//      - You will need to modify the `node_t` struct in "virtual_timer_linked_list.h"!
// 3) Place the node in the linked list
// 4) Setup the compare register so that the timer fires at the right time
// 5) Return a timer ID
//
// Your implementation will also have to take special precautions to make sure that
//  - You do not miss any timers
//  - You do not cause consistency issues in the linked list (hint: you may need the `__disable_irq()` and `__enable_irq()` functions).
//
// Follow the lab manual and start with simple cases first, building complexity and
// testing it over time.
static uint32_t timer_start(uint32_t microseconds, virtual_timer_callback_t cb, bool repeated) {

  // Return a unique timer ID. (hint: What is guaranteed unique about the timer you have created?)
  __disable_irq();
  uint32_t timer_val = read_timer();
  
  node_t* timer = malloc(sizeof(node_t)); //create node
  timer->timer_value = timer_val + microseconds; 
  timer->cb = cb;  //cb parameter would be some function
  if (repeated){
    timer->repeated = microseconds;
  }
  else{
    timer->repeated = 0;
  }
  list_insert_sorted(timer); //add node to linked list and sort
  
  //NRF_TIMER4->CC[2] = timer_val + microseconds; // expiration time, do this at the end
  
  cc_update();
  __enable_irq();
  return (uint32_t)timer; // this part is going to return an address (using malloc) which will serve as unique identifier
}

// You do not need to modify this function
// Instead, implement timer_start
uint32_t virtual_timer_start(uint32_t microseconds, virtual_timer_callback_t cb) {
  return timer_start(microseconds, cb, false);
}

// You do not need to modify this function
// Instead, implement timer_start
uint32_t virtual_timer_start_repeated(uint32_t microseconds, virtual_timer_callback_t cb) {
  return timer_start(microseconds, cb, true);
}

// Remove a timer by ID.
// Make sure you don't cause linked list consistency issues!
// Do not forget to free removed timers.
void virtual_timer_cancel(uint32_t timer_id) {
  __disable_irq();
  list_remove((timer_t*)timer_id);
  free((timer_t*)timer_id);
  cc_update();
  __enable_irq();
}

void cc_update()
{
  //get first node off list
  //if null return
  //if node --> set CC timer value
  node_t* timer = list_get_first();
  if (timer != NULL){
    NRF_TIMER4->CC[2] = timer->timer_value;
    if (read_timer() >= timer->timer_value){
      TIMER4_IRQHandler();
    }
  }
  else{
    NRF_TIMER4->CC[2] = 0; 
  }
}

