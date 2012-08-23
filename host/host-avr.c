/*
 * host-avr.c - Atmel AVR -specific HAL routines
 *
 *  Created on: Aug 21, 2012
 *      Author: csdexter
 */

#include <stdint.h>

#include <util/delay.h>

/* Masquerade as host.h, we are the HAL so it's allowed */
#define HOST_H_
#include "host/host-avr.h"


void _avr_delay_helper_ms(uint16_t ms) { while (ms--) _delay_ms(1); }
void _avr_delay_helper_us(uint16_t us) {
/* Code inspired from wiring.c of Arduino RTL */
#if F_CPU == 20000000L
  __asm__ __volatile__("nop" "\n\t" "nop");
#endif
  if (!--us) return;
#if F_CPU == 8000000L
  if (!--us) return;
#endif
  while (us--) _delay_us(1);
}
