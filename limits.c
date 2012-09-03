/*
  limits.c - code pertaining to limit-switches and performing the homing cycle
  Part of Grbl

  Copyright (c) 2009-2011 Simen Svale Skogsrud

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
#include <stdint.h>

#include "config.h"

#include "limits.h"
#include "limits-private.h"

#include "nuts_bolts.h"
#include "planner.h"
#include "settings.h"
#include "stepper.h"


void limits_init(void) {
  host_gpio_direction(LIMIT_X, HOST_GPIO_DIRECTION_INPUT, HOST_GPIO_MODE_BIT);
  host_gpio_direction(LIMIT_Y, HOST_GPIO_DIRECTION_INPUT, HOST_GPIO_MODE_BIT);
  host_gpio_direction(LIMIT_Z, HOST_GPIO_DIRECTION_INPUT, HOST_GPIO_MODE_BIT);
}

static void homing_cycle(bool x_axis, bool y_axis, bool z_axis, bool reverse_direction, uint32_t microseconds_per_pulse) {
  uint32_t step_delay = microseconds_per_pulse - settings.pulse_microseconds;
  stepper_output_t out_bits;
  limit_input_t limit_bits;

  out_bits.flags.dir_x = true;
  out_bits.flags.dir_y = true;
  out_bits.flags.dir_z = true;

  if(x_axis) out_bits.flags.step_x = true;
  if(y_axis) out_bits.flags.step_y = true;
  if(z_axis) out_bits.flags.step_z = true;

  // Invert direction bits if this is a reverse homing_cycle
  if(reverse_direction) {
    out_bits.flags.dir_x ^= true;
    out_bits.flags.dir_y ^= true;
    out_bits.flags.dir_z ^= true;
  }

  // Apply the global invert mask
  out_bits.value ^= settings.invert.masks.stepdir;

  host_gpio_write(DIR_X, out_bits.flags.dir_x, HOST_GPIO_MODE_BIT);
  host_gpio_write(DIR_Y, out_bits.flags.dir_y, HOST_GPIO_MODE_BIT);
  host_gpio_write(DIR_Z, out_bits.flags.dir_z, HOST_GPIO_MODE_BIT);

  for(;;) {
    limit_bits.flags.limit_x = host_gpio_read(LIMIT_X, HOST_GPIO_MODE_BIT);
    limit_bits.flags.limit_y = host_gpio_read(LIMIT_Y, HOST_GPIO_MODE_BIT);
    limit_bits.flags.limit_z = host_gpio_read(LIMIT_Z, HOST_GPIO_MODE_BIT);

    if(reverse_direction) {
      limit_bits.flags.limit_x ^= true;
      limit_bits.flags.limit_y ^= true;
      limit_bits.flags.limit_z ^= true;
    }

    // Apply the global invert mask
    limit_bits.value ^= settings.invert.masks.limit;

    if (x_axis && !limit_bits.flags.limit_x) {
      x_axis = false;
      out_bits.flags.step_x = false;
    }
    if (y_axis && !limit_bits.flags.limit_y) {
      y_axis = false;
      out_bits.flags.step_y = false;
    }
    if (z_axis && !limit_bits.flags.limit_z) {
      z_axis = false;
      out_bits.flags.step_z = false;
    }

    // Check if we are done
    if(!(x_axis || y_axis || z_axis)) return;

    // Send stepping pulse, can't use |= because we may have 1 -> 0 transitions,
    // e.g. when the STEP lines are inverted
    host_gpio_write(STEP_X, out_bits.flags.step_x, HOST_GPIO_MODE_BIT);
    host_gpio_write(STEP_Y, out_bits.flags.step_y, HOST_GPIO_MODE_BIT);
    host_gpio_write(STEP_Z, out_bits.flags.step_z, HOST_GPIO_MODE_BIT);
    host_delay_us(settings.pulse_microseconds);
    host_gpio_write(STEP_X, settings.invert.flags.step_x, HOST_GPIO_MODE_BIT);
    host_gpio_write(STEP_Y, settings.invert.flags.step_y, HOST_GPIO_MODE_BIT);
    host_gpio_write(STEP_Z, settings.invert.flags.step_z, HOST_GPIO_MODE_BIT);
    host_delay_us(step_delay);
  }
  return;
}

// Usually all axes have the same resolution and when that's not the case, X and
// Y have identical resolutions and Z has more -- we're looking for the slowest
// going one, i.e. the one with the least resolution, thus X makes a good
// candidate
#define FEEDRATE_TO_PERIOD_US(f) \
  ((60.0 / ((f) * settings.steps_per_mm[X_AXIS])) * 1000000)

static void approach_limit_switch(bool x, bool y, bool z) {
  homing_cycle(x, y, z, false, FEEDRATE_TO_PERIOD_US(settings.default_seek_rate) + 25); // Make it a bit slower, until we change this to the proper way (i.e. with acceleration)
}

static void leave_limit_switch(bool x, bool y, bool z) {
  homing_cycle(x, y, z, true, FEEDRATE_TO_PERIOD_US(DEFAULT_FEED) - 25); // Make this a bit faster
}

void limits_go_home() {
  plan_synchronize();
  approach_limit_switch(false, false, true); // First home the z axis
  approach_limit_switch(true, true, false);  // Then home the x and y axis
  // Now carefully leave the limit switches
  leave_limit_switch(true, true, true);
  // Conclude that this is machine zero
  sys.position[X_AXIS] = sys.position[Y_AXIS] = sys.position[Z_AXIS] = 0;
}
