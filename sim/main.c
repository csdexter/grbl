/*
  main.c - main grbl simulator program

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

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include "../planner.h"
#include "../stepper.h"
#include "../gcode.h"
#include "../protocol.h"
#include "../nuts_bolts.h"
#include "../settings.h"
#include "../spindle_control.h"
#include "../limits.h"
#include "../runtime.h"
#include "simulator.h"


int main(int argc, char *argv[]) {
  // Get the minimum time step for printing stepper values.
  // If not given or the command line cannot be parsed to a float than
  // step_time= 0.0; This means to not print stepper values at all
  if(argc>1) {
	  argv++;
	  step_time= atof(*argv);
  }

  // Setup output file handles. Block info goes to stdout. Stepper values go to stderr.
  block_out_file= stdout;
  step_out_file= stderr;
  // Make sure the output streams are flushed immediately.
  // This is important when using the simulator inside another application in parallel
  // to the real grbl.
  // Theoretically flushing could be limited to complete lines. Unfortunately Windows
  // does not know line buffered streams. So for now we stick to flushing every character.
  setvbuf(stdout, NULL, _IONBF, 1);
  setvbuf(stderr, NULL, _IONBF, 1);
  
  st_init(); // Setup stepper pins and interrupt timers
  memset(&sys, 0, sizeof(sys));  // Clear all system variables
  settings_reset();
  protocol_init(); // Clear incoming line data
  plan_init(); // Clear block buffer and planner variables
  gc_init(); // Set g-code parser to default state
  spindle_init();
  limits_init();
  st_reset(); // Clear stepper subsystem variables.
  sys.auto_start = true; // runtime commands are not processed. We start to simulate immediately
  
  // Main loop of command processing until EOF is encountered
  protocol_process();
  
  // flush the block buffer and print stepper info on any pending blocks
  plan_synchronize();

  // Graceful exit
  fclose(block_out_file);
  fclose(step_out_file);
  
  exit(EXIT_SUCCESS);
}
