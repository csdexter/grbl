/*
 * limits-private.h - definitions private to the Limits module
 *
 *  Created on: Sep 3, 2012
 *      Author: csdexter
 */

#ifndef LIMITS_PRIVATE_H_
#define LIMITS_PRIVATE_H_

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


#endif /* LIMITS_PRIVATE_H_ */
