/*
  settings.h - persistent configuration handling
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

#ifndef settings_h
#define settings_h

#include <stdbool.h>
#include <stdint.h>


#define GRBL_VERSION "0.8a"

#define SETTINGS_SIGNATURE 0x9761u

// Global settings structure
typedef struct {
  float steps_per_mm[3];
  uint8_t pulse_microseconds;
  float default_seek_rate;
  uint8_t invert_mask_stepdir;
  uint8_t invert_mask_limit;
  float mm_per_arc_segment;
  float acceleration;
  float junction_deviation;
} settings_t;

extern settings_t settings;

#define DEFAULT_FEED 60.0
#define DEFAULT_SETTINGS {{200.0, 200.0, 200.0}, 50, 600.0, 0x00, 0x00, 0.1, \
  (DEFAULT_FEED * (60 * 60)) / 10.0, 0.05 }


// Reset settings to default values
void settings_reset();

// Initialize the configuration subsystem (load settings from EEPROM)
void settings_init();

// Print current settings
void settings_dump();

// Handle settings command
uint8_t settings_execute_line(char *line);

// A helper method to set new settings from command line
void settings_store_setting(int parameter, float value);


#endif
