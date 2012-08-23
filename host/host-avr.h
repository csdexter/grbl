/*
 * host-avr.h - HAL for Atmel AVR family (8 bit)
 *
 *  Created on: Aug 21, 2012
 *      Author: csdexter
 */

#ifndef HOST_H_
# error This file must not be included directly, use host.h instead!
#endif
#ifdef HOST_HAL_H_
# error Another HAL has already been included!
#else

/* Host-specific includes */
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>


/* Host-specific interrupt enable */
#define host_sei() sei()

/* Host-specific delays */
void _avr_delay_helper_ms(uint16_t ms);
void _avr_delay_helper_us(uint16_t us);
#define host_delay_ms(ms) _avr_delay_helper_ms(ms)
#define host_delay_us(us) _avr_delay_helper_us(us)

/* Host-specific constant string treatment (literal) */
#define _S(s) PSTR(s)
/* ... (type) */
#define _S_t(s) s PROGMEM
/* ... (access method) */
#define host_fetch_S(s) pgm_read_byte(s)

#endif /* HOST_HAL_H_ */
