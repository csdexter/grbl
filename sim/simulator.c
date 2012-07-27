/*
  simulator.c - functions to simulate how the buffer is emptied and the
    stepper interrupt is called

  Part of Grbl Simulator

  Copyright (c) 2012 Jens Geisler

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

#include <stdio.h>
#include <avr/interrupt.h>
#include <inttypes.h>
#include "../stepper.h"
#include "../planner.h"
#include "../nuts_bolts.h"

// This variable is needed to determine if execute_runtime() is called in a loop
// waiting for the buffer to empty, as in plan_synchronize()
// it is reset in serial_write() because this is certainly called at the end of
// every command processing
int runtime_second_call= 0;


// Current time of the stepper simulation
double sim_time= 0.0;
// Next time the status of stepper values should be printed
double next_print_time= 0.0;
// Minimum time step for printing stepper values. Given by user via command line
double step_time= 0.0;

// global system variable structure for position etc.
system_t sys;

// Output file handles set by main program
FILE *block_out_file;
FILE *step_out_file;

// dummy port variables
uint8_t stepping_ddr;
uint8_t stepping_port;
uint8_t spindle_ddr;
uint8_t spindle_port;


// Stub of the timer interrupt function from stepper.c
void interrupt_TIMER1_COMPA_vect();

// Call the stepper interrupt until one block is finished
void sim_stepper() {
  block_t *current_block= plan_get_current_block();

  // If the block buffer is empty, call the stepper interrupt one last time
  // to let it handle sys.cycle_start etc.
  if(current_block==NULL) {
	  interrupt_TIMER1_COMPA_vect();
	  return;
  }

  while(current_block==plan_get_current_block()) {
    interrupt_TIMER1_COMPA_vect();

    // Check to see if we should print some info
    if(step_time>0.0) {
    	if(sim_time>=next_print_time) {
          fprintf(step_out_file, "  step: %20.15f, %d, %d, %d\n", sim_time, sys.position[X_AXIS], sys.position[Y_AXIS], sys.position[Z_AXIS]);

          // Make sure the simulation time doesn't get ahead of next_print_time
          while(next_print_time<sim_time) next_print_time+= step_time;
    	}
    }

    sim_time+= get_step_time();
  }
  // always print stepper values at the end of a block
  fprintf(step_out_file, "  step: %20.15f, %d, %d, %d, block_end\n", sim_time, sys.position[X_AXIS], sys.position[Y_AXIS], sys.position[Z_AXIS]);
}

// Print information about the most recently inserted block
// but only once!
void printBlock() {
  block_t *b;
  static block_t *last_block;

  b= plan_get_recent_block();
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

    last_block= b;
  }
}

// The simulator assumes that grbl is fast enough to keep the buffer full.
// Thus, the stepper interrupt is only called when the buffer is full and then only to
// finish one block.
// Only when plan_synchronize() wait for the whole buffer to clear, the stepper interrupt
// to finish all pending moves.
void handle_buffer() {
  // runtime_second_call is reset by serial_write() after every command.
  // Only when execute_runtime() is called repeatedly by plan_synchronize()
  // runtime_second_call will be incremented above 2
  if(plan_check_full_buffer() || runtime_second_call>2) {
    sim_stepper(step_out_file);
  } else {
    runtime_second_call++;
  }
}
