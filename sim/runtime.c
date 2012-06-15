/*
  protocol.c - the serial protocol master control unit
  Part of Grbl

  Copyright (c) 2009-2011 Simen Svale Skogsrud
  Copyright (c) 2011-2012 Sungeun K. Jeon  

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

// #include <avr/io.h>
// #include <inttypes.h>
// #include "../protocol.h"
// #include "../gcode.h"
// #include "../serial.h"
// #include "../print.h"
// #include "../settings.h"
// #include "../config.h"
// #include <math.h>
// #include "../nuts_bolts.h"
// #include <avr/pgmspace.h>
// #include "../stepper.h"
#include "../planner.h"
#include "sim_control.h"
#include "simstepper.h"
// #include "simstepper.h"
#include <stdio.h>

int runtime_second_call= 0;
extern FILE *block_out_file;
extern FILE *step_out_file;

// replacement for original execute_runtime as a hook to record blocks as they are generated
// and to control simulation of buffered blocks
void execute_runtime() {
  printBlock();
  handle_buffer();
}

void printBlock() {
  #define printBlockField(format, field) fprintf(block_out_file, # field ": " # format "\n", b->field)
  block_t *b;
  static block_t *last_block= NULL;
  
  b= plan_get_current_block();
  if(b!=last_block && b!=NULL) {
    fprintf(block_out_file,"  block: ");
    if(b->direction_bits & (1<<X_DIRECTION_BIT)) fprintf(block_out_file,"-");
    fprintf(block_out_file,"%d, ", b->steps_x);
    if(b->direction_bits & (1<<Y_DIRECTION_BIT)) fprintf(block_out_file,"-");
    fprintf(block_out_file,"%d, ", b->steps_y);
    if(b->direction_bits & (1<<Z_DIRECTION_BIT)) fprintf(block_out_file,"-");
    fprintf(block_out_file,"%d, ", b->steps_z);
    fprintf(block_out_file,"%f", b->nominal_speed);
    fprintf(block_out_file,"\n");
    // printBlockField(%d,  direction_bits);            // The direction bit set for this block (refers to *_DIRECTION_BIT in config.h)
    /*printBlockField(%d, steps_x);
    printBlockField(%d, steps_y);
    printBlockField(%d, steps_z);
    printBlockField(%d, step_event_count);          // The number of step events required to complete this block

    // Fields used by the motion planner to manage acceleration
    printBlockField(%f, nominal_speed);               // The nominal speed for this block in mm/min  
    printBlockField(%f, entry_speed);                 // Entry speed at previous-current block junction in mm/min
    printBlockField(%f, max_entry_speed);             // Maximum allowable junction entry speed in mm/min
    printBlockField(%f, millimeters);                 // The total travel of this block in mm
    printBlockField(%d, recalculate_flag);           // Planner flag to recalculate trapezoids on entry junction
    printBlockField(%d, nominal_length_flag);        // Planner flag for nominal speed always reached

    // Settings for the trapezoid generator
    printBlockField(%d, initial_rate);              // The step rate at start of block  
    printBlockField(%d, final_rate);                // The step rate at end of block
    printBlockField(%d, rate_delta);                 // The steps/minute to add or subtract when changing speed (must be positive)
    printBlockField(%d, accelerate_until);          // The index of the step event on which to stop acceleration
    printBlockField(%d, decelerate_after);          // The index of the step event on which to start decelerating
    printBlockField(%d, nominal_rate);              // The nominal step rate for this block in step_events/minute
    fprintf(block_out_file,"\n");*/
    last_block= b;
  }
}

void handle_buffer() {
  if(plan_check_full_buffer() || runtime_second_call) {
    sim_stepper(step_out_file);
  } else {
    runtime_second_call= 1;
  }
}