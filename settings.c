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
#include "print.h"
#include "protocol.h"


settings_t settings;

void settings_reset() {
  const settings_t defaults = DEFAULT_SETTINGS;

  settings = defaults;
}

void settings_dump() {
  printMessage(_S("$0 = ")); printFloat(settings.steps_per_mm[X_AXIS]);
  printMessage(_S(" (steps/mm x)\r\n$1 = ")); printFloat(settings.steps_per_mm[Y_AXIS]);
  printMessage(_S(" (steps/mm y)\r\n$2 = ")); printFloat(settings.steps_per_mm[Z_AXIS]);
  printMessage(_S(" (steps/mm z)\r\n$3 = ")); printInteger(settings.pulse_microseconds);
  printMessage(_S(" (microseconds step pulse)\r\n$4 = ")); printFloat(settings.default_seek_rate);
  printMessage(_S(" (mm/min default seek rate)\r\n$5 = ")); printFloat(settings.mm_per_arc_segment);
  printMessage(_S(" (mm/arc segment)\r\n$6 = ")); printInteger(settings.invert_mask_stepdir);
  printMessage(_S(" (step port invert mask. binary = ")); printBinary(settings.invert_mask_stepdir);
  printMessage(_S(")\r\n$7 = ")); printInteger(settings.invert_mask_limit);
  printMessage(_S(" (limits port invert mask. binary = ")); printBinary(settings.invert_mask_limit);
  printMessage(_S(")\r\n$8 = ")); printFloat(settings.acceleration/(60*60)); // Convert from mm/min^2 for human readability
  printMessage(_S(" (acceleration in mm/sec^2)\r\n$9 = ")); printFloat(settings.junction_deviation);
  printMessage(_S(" (cornering junction deviation in mm)"));
  printMessage(_S("\r\n'$x=value' to set parameter or just '$' to dump current settings\r\n"));
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
      printMessage(_S("Steps/mm must be > 0.0\r\n"));
      return;
    }
    settings.steps_per_mm[parameter] = value; break;
    case 3: 
    if (value < 3) {
      printMessage(_S("Step pulse must be >= 3 microseconds\r\n"));
      return;
    }
    settings.pulse_microseconds = round(value); break;
    case 4: settings.default_seek_rate = value; break;
    case 5: settings.mm_per_arc_segment = value; break;
    case 6: settings.invert_mask_stepdir = trunc(value); break;
    case 7: settings.invert_mask_limit = trunc(value); break;
    case 8: settings.acceleration = value * 60 * 60; break; // Convert to mm/min^2 for grbl internal use.
    case 9: settings.junction_deviation = fabs(value); break;
    default: printMessage(_S("Unknown parameter\r\n")); return;
  }
  host_settings_store(SETTINGS_SIGNATURE, &settings, sizeof(settings));
  printMessage(_S("Stored new setting\r\n"));
}

// Initialize the config subsystem
void settings_init() {
  if(host_settings_fetch(SETTINGS_SIGNATURE, &settings, sizeof(settings)) != HOST_SETTING_OK) {
    printMessage(_S("Warning: Failed to read EEPROM settings. Using defaults.\r\n"));
    settings_reset();
    host_settings_store(SETTINGS_SIGNATURE, &settings, sizeof(settings));
    settings_dump();
  }
}
