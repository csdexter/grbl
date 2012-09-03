/*
  spindle_control.c - spindle control methods
  Part of Grbl

  Copyright (c) 2009-2011 Simen Svale Skogsrud
  Copyright (c) 2012 Sungeun K. Jeon

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

#include <stdint.h>

#include "config.h"

#include "planner.h"


static uint8_t current_direction;

void spindle_stop(void) {
  /* If we were spinning, we need to wait until all previous moves are executed
   * before stopping. */
  if(current_direction) plan_synchronize();
  host_gpio_write(SPINDLE_ENABLE, false, HOST_GPIO_MODE_BIT);
}

void spindle_init(void) {
  current_direction = 0;
  host_gpio_direction(SPINDLE_ENABLE, HOST_GPIO_DIRECTION_OUTPUT, HOST_GPIO_MODE_BIT);
  #ifdef SPINDLE_DIRECTION
    host_gpio_direction(SPINDLE_DIRECTION, HOST_GPIO_DIRECTION_OUTPUT, HOST_GPIO_MODE_BIT);
  #endif
  spindle_stop();
}

void spindle_run(int direction, uint32_t rpm) 
{
  /* If we need to change state, we must wait for all moves to complete before
   * doing so. */
  if (direction != current_direction) {
    plan_synchronize();
    if (direction) {
      #ifdef SPINDLE_DIRECTION
        if(direction > 0)
          host_gpio_write(SPINDLE_DIRECTION, false, HOST_GPIO_MODE_BIT);
        else host_gpio_write(SPINDLE_DIRECTION, true, HOST_GPIO_MODE_BIT);
      #endif
      host_gpio_write(SPINDLE_ENABLE, true, HOST_GPIO_MODE_BIT);
    } else {
      spindle_stop();
    }
    current_direction = direction;
  }
}
