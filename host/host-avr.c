/*
 * host-avr.c - Atmel AVR -specific routines
 *
 *  Created on: Aug 21, 2012
 *      Author: csdexter
 */

#include <stdint.h>

#include <util/delay.h>

/* Masquerade as host.h, we are the HAL so it's allowed */
#define HOST_H_
#include "host/host-avr.h"


inline void _avr_delay_helper_ms(uint16_t ms) { while (ms--) _delay_ms(1); }
inline void _avr_delay_helper_us(uint16_t us) { while (us--) _delay_us(1); }
