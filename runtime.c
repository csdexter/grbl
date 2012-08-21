/*
  runtime.c - run time command handling part of grbl

  Copyright (c) 2009-2011 Simen Svale Skogsrud
  Copyright (c) 2011-2012 Sungeun K. Jeon
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

#include <stdbool.h>
#include <string.h>

#include "config.h"

#include "nuts_bolts.h"
#include "print.h"
#include "settings.h"
#include "stepper.h"


void runtime_status_report()
{
 // TODO: Status report data is written to the user here. This function should be able to grab a
 // real-time snapshot of the stepper subprogram and the actual location of the CNC machine. At a
 // minimum, status report should return real-time location information. Other important information
 // may be distance to go on block, processed block id, and feed rate. A secondary, non-critical
 // status report may include g-code state, i.e. inch mode, plane mode, absolute mode, etc.
 //   The report generated must be as short as possible, yet still provide the user easily readable
 // information, i.e. 'x0.23,y120.4,z2.4'. This is necessary as it minimizes the computational
 // overhead and allows grbl to keep running smoothly, especially with g-code programs with fast,
 // short line segments and interface setups that require real-time status reports (5-20Hz).

 // **Under construction** Bare-bones status report. Provides real-time machine position relative to
 // the system power on location (0,0,0) and work coordinate position (G54 and G92 applied).
 // The following are still needed: user setting of output units (mm|inch), compressed (non-human
 // readable) data for interfaces?, save last known position in EEPROM?, code optimizations, solidify
 // the reporting schemes, move to a separate .c file for easy user accessibility, and setting the
 // home position by the user (likely through '$' setting interface).
 // Successfully tested at a query rate of 10-20Hz while running a gauntlet of programs at various
 // speeds.
 int32_t print_position[3];
 memcpy(print_position,sys.position,sizeof(sys.position));
 #if REPORT_INCH_MODE
   printString("MPos:["); printFloat(print_position[X_AXIS]/(settings.steps_per_mm[X_AXIS]*MM_PER_INCH));
   printString(","); printFloat(print_position[Y_AXIS]/(settings.steps_per_mm[Y_AXIS]*MM_PER_INCH));
   printString(","); printFloat(print_position[Z_AXIS]/(settings.steps_per_mm[Z_AXIS]*MM_PER_INCH));
   printString("],WPos:["); printFloat((print_position[X_AXIS]/settings.steps_per_mm[X_AXIS]-sys.coord_system[sys.coord_select][X_AXIS]-sys.coord_offset[X_AXIS])/MM_PER_INCH);
   printString(","); printFloat((print_position[Y_AXIS]/settings.steps_per_mm[Y_AXIS]-sys.coord_system[sys.coord_select][Y_AXIS]-sys.coord_offset[Y_AXIS])/MM_PER_INCH);
   printString(","); printFloat((print_position[Z_AXIS]/settings.steps_per_mm[Z_AXIS]-sys.coord_system[sys.coord_select][Z_AXIS]-sys.coord_offset[Z_AXIS])/MM_PER_INCH);
 #else
   printString("MPos:["); printFloat(print_position[X_AXIS]/(settings.steps_per_mm[X_AXIS]));
   printString(","); printFloat(print_position[Y_AXIS]/(settings.steps_per_mm[Y_AXIS]));
   printString(","); printFloat(print_position[Z_AXIS]/(settings.steps_per_mm[Z_AXIS]));
   printString("],WPos:["); printFloat(print_position[X_AXIS]/settings.steps_per_mm[X_AXIS]-sys.coord_system[sys.coord_select][X_AXIS]-sys.coord_offset[X_AXIS]);
   printString(","); printFloat(print_position[Y_AXIS]/settings.steps_per_mm[Y_AXIS]-sys.coord_system[sys.coord_select][Y_AXIS]-sys.coord_offset[Y_AXIS]);
   printString(","); printFloat(print_position[Z_AXIS]/settings.steps_per_mm[Z_AXIS]-sys.coord_system[sys.coord_select][Z_AXIS]-sys.coord_offset[Z_AXIS]);
 #endif
 printString("]\r\n");
}

// Executes run-time commands, when required. This is called from various check points in the main
// program, primarily where there may be a while loop waiting for a buffer to clear space or any
// point where the execution time from the last check point may be more than a fraction of a second.
// This is a way to execute runtime commands asynchronously (aka multitasking) with grbl's g-code
// parsing and planning functions. This function also serves as an interface for the interrupts to 
// set the system runtime flags, where only the main program to handles them, removing the need to
// define more computationally-expensive volatile variables.
// NOTE: The sys.execute variable flags are set by the serial read subprogram, except where noted.
void execute_runtime()
{
  if (sys.execute) { // Enter only if any bit flag is true
    uint8_t rt_exec = sys.execute; // Avoid calling volatile multiple times
  
    // System abort. Steppers have already been force stopped.
    if (rt_exec & EXEC_RESET) {
      sys.abort = true; 
      return; // Nothing else to do but exit.
    }
    
    // Execute and serial print status
    if (rt_exec & EXEC_STATUS_REPORT) { 
      runtime_status_report();
      bit_false(sys.execute,EXEC_STATUS_REPORT);
    }
    
    // Initiate stepper feed hold
    if (rt_exec & EXEC_FEED_HOLD) {
      st_feed_hold(); // Initiate feed hold.
      bit_false(sys.execute,EXEC_FEED_HOLD);
    }
    
    // Reinitializes the stepper module running flags and re-plans the buffer after a feed hold.
    // NOTE: EXEC_CYCLE_STOP is set by the stepper subsystem when a cycle or feed hold completes.
    if (rt_exec & EXEC_CYCLE_STOP) {
      st_cycle_reinitialize();
      bit_false(sys.execute,EXEC_CYCLE_STOP);
    }
    
    if (rt_exec & EXEC_CYCLE_START) { 
      st_cycle_start(); // Issue cycle start command to stepper subsystem
      #ifdef CYCLE_AUTO_START
        sys.auto_start = true; // Re-enable auto start after feed hold.
      #endif
      bit_false(sys.execute,EXEC_CYCLE_START);
    } 
  }
}  
