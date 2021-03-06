/*
 * stepper-private.h - private definitions for the Stepper module
 *
 *  Created on: Sep 2, 2012
 *      Author: csdexter
 */

#ifndef STEPPER_PRIVATE_H_
#define STEPPER_PRIVATE_H_

#include <stdint.h>

#include "config.h"

// Some useful constants
#define TICKS_PER_MICROSECOND (HOST_TIMER_FOSC / 1000000)
#define CYCLES_PER_ACCELERATION_TICK (HOST_TIMER_FOSC / ACCELERATION_TICKS_PER_SECOND)

// Stepper state variable. Contains running data and trapezoid variables.
typedef struct {
  // Used by Bresenham's line algorithm
  int32_t counter_x,               // Counter variables for Bresenham's line tracer
          counter_y,
          counter_z;
  uint32_t event_count;
  uint32_t step_events_completed;  // The number of step events left in current motion
  // Used by the trapezoid generator
  uint32_t cycles_per_step_event;        // The number of machine cycles between each step event
  uint32_t trapezoid_tick_cycle_counter; // The cycles since last trapezoid_tick, used to generate ticks at a steady pace without allocating a separate timer
  uint32_t trapezoid_adjusted_rate;      // The current rate of step_events according to the trapezoid generator
  uint32_t min_safe_rate;                // Minimum safe rate for full deceleration rate reduction step, otherwise halves step_rate.
} stepper_t;

// Local functions
static void set_step_events_per_minute(uint32_t steps_per_minute);
static void st_wake_up(void);
static uint8_t iterate_trapezoid_cycle_counter(void);


#endif /* STEPPER_PRIVATE_H_ */
