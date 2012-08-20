/*
  stepper.h - stepper motor driver: executes motion plans of planner.c using the stepper motors
  Part of Grbl

  Copyright (c) 2009-2011 Simen Svale Skogsrud
  Copyright (c) 2011 Sungeun K. Jeon

  Grbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Grbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Grbl.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef stepper_h
#define stepper_h 

#include <avr/io.h>

// Some useful constants
#define STEP_MASK (_BV(X_STEP_BIT) | _BV(Y_STEP_BIT) | _BV(Z_STEP_BIT)) // All step bits
#define DIRECTION_MASK (_BV(X_DIRECTION_BIT) | _BV(Y_DIRECTION_BIT) | _BV(Z_DIRECTION_BIT)) // All direction bits
#define STEPPING_MASK (STEP_MASK | DIRECTION_MASK) // All stepping-related bits (step/direction)

// Initialize and setup the stepper motor subsystem
void st_init();

// Immediately disables steppers
void st_go_idle();

// Reset the stepper subsystem variables
void st_reset();

// Notify the stepper subsystem to start executing the g-code program in buffer.
void st_cycle_start();

// Reinitializes the buffer after a feed hold for a resume.
void st_cycle_reinitialize(); 

// Initiates a feed hold of the running program
void st_feed_hold();

// Time between step events used in the simulator.
// Accessor function to keep st struct static
double get_step_time();

#endif
