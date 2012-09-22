/*
  coolant_control.c - coolant control methods
  Part of Grbl

  Copyright (c) 2012 Sungeun K. Jeon

  Grbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Grbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Grbl. If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdint.h>

#include "config.h"

#include "coolant_control.h"

#include "planner.h"


static uint8_t current_coolant_mode;

void coolant_init(void) {
  current_coolant_mode = COOLANT_OFF;
  #ifdef CSPRAY_ENABLE
    host_gpio_direction(CSPRAY_ENABLE, HOST_GPIO_DIRECTION_OUTPUT, HOST_GPIO_MODE_BIT);
  #endif
  #ifdef CFLOOD_ENABLE
    host_gpio_direction(CFLOOD_ENABLE, HOST_GPIO_DIRECTION_OUTPUT, HOST_GPIO_MODE_BIT);
  #endif
  coolant_stop();
}

void coolant_stop(void) {
  /* If coolant was running, we need to wait until all previous moves are
   * executed before turning it off. */
  if(current_coolant_mode) plan_synchronize();

  #ifdef CSPRAY_ENABLE
    host_gpio_write(CSPRAY_ENABLE, false, HOST_GPIO_MODE_BIT);
  #endif
  #ifdef CFLOOD_ENABLE
    host_gpio_write(CFLOOD_ENABLE, false, HOST_GPIO_MODE_BIT);
  #endif
}


void coolant_run(uint8_t mode) {
  /* If we need to change state, we must wait for all moves to complete before
   * doing so. */
  if(mode != current_coolant_mode) {
    plan_synchronize();
    if(mode != COOLANT_OFF) {
      #ifdef CSPRAY_ENABLE
        if(mode & COOLANT_MIST)
          host_gpio_write(CSPRAY_ENABLE, true, HOST_GPIO_MODE_BIT);
      #endif
      #ifdef CFLOOD_ENABLE
        if(mode & COOLANT_FLOOD)
          host_gpio_write(CFLOOD_ENABLE, true, HOST_GPIO_MODE_BIT);
      #endif
    } else coolant_stop();
    current_coolant_mode = mode;
  }
}
