/*
  settings.c - eeprom configuration handling 
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

#include <math.h>
#include <stdbool.h>

#include "config.h"

#include "settings.h"

#include "nuts_bolts.h"
#include "protocol.h"


settings_t settings;

void settings_reset(void) {
  const settings_t defaults = DEFAULT_SETTINGS;

  settings = defaults;
}

void settings_dump(void) {
  host_serialconsole_printmessage(_S("$0 = "), true);
  host_serialconsole_printfloat(settings.steps_per_mm[X_AXIS], 4, true);
  host_serialconsole_printmessage(_S(" (steps/mm x)\r\n$1 = "), true);
  host_serialconsole_printfloat(settings.steps_per_mm[Y_AXIS], 4, true);
  host_serialconsole_printmessage(_S(" (steps/mm y)\r\n$2 = "), true);
  host_serialconsole_printfloat(settings.steps_per_mm[Z_AXIS], 4, true);
  host_serialconsole_printmessage(_S(" (steps/mm z)\r\n$3 = "), true);
  host_serialconsole_printinteger(settings.pulse_microseconds, true);
  host_serialconsole_printmessage(_S(" (microseconds step pulse)\r\n$4 = "), true);
  host_serialconsole_printfloat(settings.default_seek_rate, 2, true);
  host_serialconsole_printmessage(_S(" (mm/min default seek rate)\r\n$5 = "), true);
  host_serialconsole_printfloat(settings.mm_per_arc_segment, 4, true);
  host_serialconsole_printmessage(_S(" (mm/arc segment)\r\n$6 = "), true);
  host_serialconsole_printinteger(settings.invert.mask, true);
  host_serialconsole_printmessage(_S(" (GPIO port invert mask. binary = "), true);
  host_serialconsole_printbinary((settings.invert.mask >> 8), true);
  host_serialconsole_printbinary((settings.invert.mask & 0x00FFU), true);
  host_serialconsole_printmessage(_S(")\r\n$7 = "), true);
  host_serialconsole_printfloat(settings.acceleration / (60 * 60), 2, true); // Convert from mm/min^2 for human readability
  host_serialconsole_printmessage(_S(" (acceleration in mm/sec^2)\r\n$8 = "), true);
  host_serialconsole_printfloat(settings.junction_deviation, 4, true);
  host_serialconsole_printmessage(_S(" (cornering junction deviation in mm)"), true);
  host_serialconsole_printmessage(_S("\r\n'$x=value' to set parameter or just '$' to dump current settings\r\n"), true);
}

// Parameter lines are on the form '$4=374.3' or '$' to dump current settings
uint8_t settings_execute_line(char *line) {
  uint8_t char_counter = 1;
  float parameter, value;

  if(line[0] != '$') { 
    return (STATUS_UNSUPPORTED_STATEMENT);
  }
  if(!line[char_counter]) {
    settings_dump();
    return (STATUS_OK);
  }
  if(!read_float(line, &char_counter, &parameter)) {
    return(STATUS_BAD_NUMBER_FORMAT);
  };
  if(line[char_counter++] != '=') { 
    return (STATUS_UNSUPPORTED_STATEMENT);
  }
  if(!read_float(line, &char_counter, &value)) {
    return(STATUS_BAD_NUMBER_FORMAT);
  }
  if(line[char_counter] != 0) { 
    return (STATUS_UNSUPPORTED_STATEMENT);
  }
  settings_store_setting(parameter, value);
  return (STATUS_OK);
}

// A helper method to set settings from command line
void settings_store_setting(int parameter, float value) {
  switch(parameter) {
    case 0: case 1: case 2:
    if (value <= 0.0) {
      host_serialconsole_printmessage(_S("Steps/mm must be > 0.0\r\n"), true);
      return;
    }
    settings.steps_per_mm[parameter] = value; break;
    case 3: 
    if (value < 3) {
      host_serialconsole_printmessage(_S("Step pulse must be >= 3 microseconds\r\n"), true);
      return;
    }
    settings.pulse_microseconds = round(value); break;
    case 4: settings.default_seek_rate = value; break;
    case 5: settings.mm_per_arc_segment = value; break;
    case 6: settings.invert.mask = trunc(value); break;
    case 7: settings.acceleration = value * 60 * 60; break; // Convert to mm/min^2 for grbl internal use.
    case 8: settings.junction_deviation = fabs(value); break;
    default: host_serialconsole_printmessage(_S("Unknown parameter\r\n"), true); return;
  }
  host_settings_store(SETTINGS_SIGNATURE, &settings, sizeof(settings));
  host_serialconsole_printmessage(_S("Stored new setting\r\n"), true);
}

// Initialize the config subsystem
void settings_init(void) {
  if(host_settings_fetch(SETTINGS_SIGNATURE, &settings, sizeof(settings)) != HOST_SETTING_OK) {
    host_serialconsole_printmessage(_S("Warning: Failed to read EEPROM settings. Using defaults.\r\n"), true);
    settings_reset();
    host_settings_store(SETTINGS_SIGNATURE, &settings, sizeof(settings));
    settings_dump();
  }
}
