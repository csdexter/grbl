/*
  nuts_bolts.c - Shared functions
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
#include <stdlib.h>

#include "config.h"


bool read_float(char *line, uint8_t *char_counter, float *float_ptr) {
  char *end, saveChar = '\0';
  uint8_t saveIndex = *char_counter;

  if(line[saveIndex] == '+' || line[saveIndex] == '-') saveIndex++; // Skip over sign, if present
  while(line[saveIndex] && ((line[saveIndex] >= '0' && line[saveIndex] <= '9') || line[saveIndex] == '.')) saveIndex++; // Find the end of the number
  if(line[saveIndex]) { // Otherwise it's \0 anyway, no need to fixup
    saveChar = line[saveIndex];
    line[saveIndex] = '\0';
  }
  
  // strtod() will now see only the number and nothing beyond it
  *float_ptr = strtod(&line[*char_counter], &end);
  if(saveChar) line[saveIndex] = saveChar; // Undo the fix, if needed
  if(end == &line[*char_counter]) return false;
  else {
    *char_counter = end - line;
    return true;
  }
}
