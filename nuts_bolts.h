/*
  nuts_bolts.h - Header file for shared definitions, variables, and functions
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

#ifndef nuts_bolts_h
#define nuts_bolts_h

#include <stdbool.h>
#include <stdint.h>


#define X_AXIS 0
#define Y_AXIS 1
#define Z_AXIS 2

#define MM_PER_INCH (25.4)

// Useful macros
#define clear_vector(a) memset(a, 0, sizeof(a))
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))

// Bit field and masking macros
#define bit(n) (1 << n) 
#define bit_true(x,mask) (x |= mask)
#define bit_false(x,mask) (x &= ~mask)
#define bit_istrue(x,mask) ((bool)(x & mask))

// Define system executor bit map. Used internally by runtime protocol as
// runtime command flags, which notifies the main program to execute the
// specified runtime command asynchronously.
// NOTE: The system executor uses an unsigned 8-bit volatile variable (8 flag
// limit). The default flags are always false, so the runtime protocol only
// needs to check for a non-zero value to know when there is a runtime command
// to execute.
// #define                  bit(0) // bitmask 00000001
#define EXEC_CYCLE_START    bit(1) // bitmask 00000010
#define EXEC_CYCLE_STOP     bit(2) // bitmask 00000100
#define EXEC_FEED_HOLD      bit(3) // bitmask 00001000
#define EXEC_RESET          bit(4) // bitmask 00010000
// #define                  bit(5) // bitmask 00100000
// #define                  bit(6) // bitmask 01000000
// #define                  bit(7) // bitmask 10000000

// Define global system variables
typedef struct {
  uint8_t abort:1;               // System abort flag. Forces exit back to main loop for reset.
  uint8_t feed_hold:1;           // Feed hold flag. Held true during feed hold. Released when ready to resume.
  uint8_t auto_start:1;          // Planner auto-start flag. Toggled off during feed hold. Defaulted by settings.
  uint8_t reserved_flags1:5;     // Hold the other 5 bits, make sure gcc doesn't get ideas about them
  int32_t position[3];           // Real-time machine (aka home) position vector in steps. 
                                 // NOTE: This may need to be a volatile variable, if problems arise. 
  uint8_t coord_select;          // Active work coordinate system number. Default: 0=G54.
  float coord_system[N_COORDINATE_SYSTEM][3]; // Work coordinate systems (G54+). Stores offset from absolute machine position in mm.
                                 // Rows: Work system number (0=G54,1=G55,...5=G59), Columns: XYZ Offsets
  float coord_offset[3];         // Retains the G92 coordinate offset (work coordinates) relative to machine zero in mm.
  volatile uint8_t cycle_start;  // Cycle start flag. Set by stepper subsystem or main program.
  volatile uint8_t execute;      // Global system runtime executor bit flag variable. See EXEC bitmasks.
} system_t;
extern system_t sys;

// Read a floating point value from a string. Line points to the input buffer,
// char_counter is the index of the current character on the line (i.e. where
// conversion should start) while float_ptr is a pointer to the result.
// Returns true on success, false otherwise. On success, char_counter is updated
// to point after the value read from the line.
bool read_float(char *line, uint8_t *char_counter, float *float_ptr);


#endif
