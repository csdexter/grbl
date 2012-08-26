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

#include <stdbool.h>

#include "config.h"

#include "protocol.h"

#include "gcode.h"
#include "nuts_bolts.h"
#include "print.h"
#include "runtime.h"
#include "settings.h"


#define LINE_BUFFER_SIZE 50

static char line[LINE_BUFFER_SIZE]; // Line to be executed. Zero-terminated.
static uint8_t char_counter; // Last character counter in line variable.
static uint8_t iscomment; // Comment/block delete flag for processor to ignore comment characters.

static void status_message(int status_code) 
{
  if (status_code == 0) {
    host_serialconsole_printmessage(_S("ok\r\n"), true);
  } else {
    host_serialconsole_printmessage(_S("error: "), true);
    switch(status_code) {          
      case STATUS_BAD_NUMBER_FORMAT:
      host_serialconsole_printmessage(_S("Bad number format\r\n"), true); break;
      case STATUS_EXPECTED_COMMAND_LETTER:
      host_serialconsole_printmessage(_S("Expected command letter\r\n"), true); break;
      case STATUS_UNSUPPORTED_STATEMENT:
      host_serialconsole_printmessage(_S("Unsupported statement\r\n"), true); break;
      case STATUS_FLOATING_POINT_ERROR:
      host_serialconsole_printmessage(_S("Floating point error\r\n"), true); break;
      case STATUS_MODAL_GROUP_VIOLATION:
      host_serialconsole_printmessage(_S("Modal group violation\r\n"), true); break;
      case STATUS_INVALID_COMMAND:
      host_serialconsole_printmessage(_S("Invalid command\r\n"), true); break;
      default:
      printInteger(status_code);
      host_serialconsole_printmessage(_S("\r\n"), true);
    }
  }
}

void protocol_init() 
{
  // Print grbl initialization message
  host_serialconsole_printmessage(_S("\r\nGrbl " GRBL_VERSION), true);
  host_serialconsole_printmessage(_S("\r\n'$' to dump current settings\r\n"), true);

  char_counter = 0; // Reset line input
  iscomment = false;
}


// Executes one line of input according to protocol
uint8_t protocol_execute_line(char *line) 
{     
  if(line[0] == '$') {
  
    // TODO: Re-write this '$' as a way to change runtime settings without having to reset, i.e.
    // auto-starting, status query output formatting and type, jog mode (axes, direction, and
    // nominal feedrate), toggle block delete, etc. This differs from the EEPROM settings, as they
    // are considered defaults and loaded upon startup/reset.
    //   This use is envisioned where '$' itself dumps settings and help. Defined characters
    // proceeding the '$' may be used to setup modes, such as jog mode with a '$J=X100' for X-axis
    // motion with a nominal feedrate of 100mm/min. Writing EEPROM settings will likely stay the 
    // same or similar. Should be worked out in upcoming releases.    
    return(settings_execute_line(line)); // Delegate lines starting with '$' to the settings module

  // } else if { 
  //
  // JOG MODE
  //
  // TODO: Here jogging can be placed for execution as a seperate subprogram. It does not need to be 
  // susceptible to other runtime commands except for e-stop. The jogging function is intended to
  // be a basic toggle on/off with controlled acceleration and deceleration to prevent skipped 
  // steps. The user would supply the desired feedrate, axis to move, and direction. Toggle on would
  // start motion and toggle off would initiate a deceleration to stop. One could 'feather' the
  // motion by repeatedly toggling to slow the motion to the desired location. Location data would 
  // need to be updated real-time and supplied to the user through status queries.
  //   More controlled exact motions can be taken care of by inputting G0 or G1 commands, which are 
  // handled by the planner. It would be possible for the jog subprogram to insert blocks into the
  // block buffer without having the planner plan them. It would need to manage de/ac-celerations 
  // on its own carefully. This approach could be effective and possibly size/memory efficient.

  } else {
    return(gc_execute_line(line));    // Everything else is gcode
  }
}


// Process one line of incoming serial data. Remove unneeded characters and capitalize.
void protocol_process()
{
  uint8_t c;
  while((c = host_serialconsole_read()) != CONSOLE_NO_DATA) {
    if ((c == '\n') || (c == '\r')) { // End of line reached

      // Runtime command check point before executing line. Prevent any further line executions.
      // NOTE: If there is no line, this function should quickly return to the main program when
      // the buffer empties of non-executable data.
      execute_runtime();
      if (sys.abort) { return; } // Bail to main program upon system abort    

      if (char_counter > 0) {// Line is complete. Then execute!
        line[char_counter] = 0; // Terminate string
        status_message(protocol_execute_line(line));
      } else { 
        // Empty or comment line. Skip block.
        status_message(STATUS_OK); // Send status message for syncing purposes.
      }
      char_counter = 0; // Reset line buffer index
      iscomment = false; // Reset comment flag
      
    } else {
      if (iscomment) {
        // Throw away all comment characters
        if (c == ')') {
          // End of comment. Resume line.
          iscomment = false;
        }
      } else {
        if (c <= ' ') { 
          // Throw away whitepace and control characters
        } else if (c == '/') {
          // Disable block delete and throw away characters. Will ignore until EOL.
          #if BLOCK_DELETE_ENABLE
            iscomment = true;
          #endif
        } else if (c == '(') {
          // Enable comments flag and ignore all characters until ')' or EOL.
          iscomment = true;
        } else if (char_counter >= LINE_BUFFER_SIZE-1) {
          // Throw away any characters beyond the end of the line buffer
        } else if (c >= 'a' && c <= 'z') { // Upcase lowercase
          line[char_counter++] = c-'a'+'A';
        } else {
          line[char_counter++] = c;
        }
      }
    }
  }
}
