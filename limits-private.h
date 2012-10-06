/*
 * limits-private.h - definitions private to the Limits module
 *
 *  Created on: Sep 3, 2012
 *      Author: csdexter
 */

#ifndef LIMITS_PRIVATE_H_
#define LIMITS_PRIVATE_H_

#include <stdbool.h>
#include <stdint.h>


typedef union {
  struct {
    uint8_t limit_x:1;
    uint8_t limit_y:1;
    uint8_t limit_z:1;
    uint8_t reserved:6;
  } flags;
  uint8_t value;
} limit_input_t;

// Local functions
static void homing_cycle(bool x_axis, bool y_axis, bool z_axis, bool reverse_direction, uint32_t microseconds_per_pulse);
static void approach_limit_switch(bool x, bool y, bool z);
static void leave_limit_switch(bool x, bool y, bool z);


#endif /* LIMITS_PRIVATE_H_ */
