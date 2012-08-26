/*
 * host-avr.c - Atmel AVR-specific HAL routines
 *
 *  Created on: Aug 21, 2012
 *      Author: csdexter
 *
 * NOTE: this file is tuned for the ATmega328p that you can usually find
 *       powering the Arduinos. If another 8-bit AVR is targeted, conditional
 *       defines should be inserted to make the code portable among sub-
 *       architectures.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <avr/io.h>
#include <avr/power.h>
#include <util/delay.h>
#include <util/delay_basic.h>

#include "host.h"


void host_delay_ms(uint16_t ms) { while (ms--) _delay_ms(1); }
void host_delay_us(uint16_t us) {
/* Code taken from wiring.c of Arduino RTL */
#if F_CPU == 20000000L
  __asm__ __volatile__("nop" "\n\t" "nop");
  if(!--us) return;
  us = (us << 2) + us; // Faster than multiplying by 5
  us -= 2; // account for the time taken by the previous lines
#elif F_CPU == 16000000L
  if(!--us) return;
  us <<= 2; // Faster than multiplying by 4
  us -= 2; // account for the time taken by the previous lines
#elif F_CPU == 8000000L
  if(!--us) return;
  if(!--us) return;
  us <<= 1; // Faster than multiplying by 2
  us--;
#else
# error F_CPU undefined or not in {8, 16, 20}MHz
#endif
  _delay_loop_2(us);
}

static char serialconsole_rx_buffer[CONSOLE_RXBUF_SIZE];
static volatile uint8_t serialconsole_rx_buffer_head = 0;
static uint8_t serialconsole_rx_buffer_tail = 0;
static char serialconsole_tx_buffer[CONSOLE_TXBUF_SIZE];
static uint8_t serialconsole_tx_buffer_head = 0;
static volatile uint8_t serialconsole_tx_buffer_tail = 0;

void host_serialconsole_init(uint16_t baud) {
#define BAUD CONSOLE_BAUD_RATE
#include <util/setbaud.h>

  power_usart0_enable();

  UBRR0H = UBRRH_VALUE;
  UBRR0L = UBRRL_VALUE;
#if USE_2X
  UCSR0A |= _BV(U2X0);
#else
  UCSR0A &= ~_BV(U2X0);
#endif
  UCSR0B |= _BV(RXEN0) | _BV(TXEN0) | _BV(RXCIE0);
}

void host_serialconsole_reset() {
  serialconsole_rx_buffer_tail = serialconsole_rx_buffer_head;
}

char host_serialconsole_read(void) {
  if(serialconsole_rx_buffer_head == serialconsole_rx_buffer_tail)
    return CONSOLE_NO_DATA;
  else {
    uint8_t data = serialconsole_rx_buffer[serialconsole_rx_buffer_tail];
    if (++serialconsole_rx_buffer_tail == CONSOLE_RXBUF_SIZE)
      serialconsole_rx_buffer_tail = 0;

    return data;
  }
}

bool host_serialconsole_write(char c, bool block) {
  uint8_t new_head =
      ((serialconsole_tx_buffer_head + 1) == CONSOLE_TXBUF_SIZE)
      ? 0 : serialconsole_tx_buffer_head + 1;

  while (new_head == serialconsole_tx_buffer_tail)
    if(!block) return false;

  serialconsole_tx_buffer[serialconsole_tx_buffer_head] = c;
  serialconsole_tx_buffer_head = new_head;

  UCSR0B |= _BV(UDRIE0);

  return true;
}

bool host_serialconsole_printinteger(uint32_t n, bool block) {
  char buf[12]; /* 4294967295 + 1 for sign + 1 for \0 */

  return host_serialconsole_printstring(ultoa(n, buf, 10), block);
}

bool host_serialconsole_printbinary(uint8_t n, bool block) {
  uint8_t i;

  for(i = 8; i; i--) {
    if(!host_serialconsole_write('0' + (bool)(n & 0x80), block)) return false;
    n <<= 1;
  }

  return true;
}

bool host_serialconsole_printfloat(float n, uint8_t precision, bool block) {
  char buf[11]; /* 8 digits + 1 for sign + 1 for decimal point + 1 for \0 */

  return host_serialconsole_printstring(dtostrf(n, 0, precision, buf), block);
}

ISR(USART_UDRE_vect) {
  UDR0 = serialconsole_tx_buffer[serialconsole_tx_buffer_tail];

  if(++serialconsole_tx_buffer_tail == CONSOLE_TXBUF_SIZE)
    serialconsole_tx_buffer_tail = 0;
  if(serialconsole_tx_buffer_tail == serialconsole_tx_buffer_head)
    UCSR0B &= ~_BV(UDRIE0);
}

ISR(USART_RX_vect) {
  /* Need to actually perform the read to clear "data received" status */
  char data = UDR0;
  uint8_t new_head = ((serialconsole_rx_buffer_head + 1) == CONSOLE_RXBUF_SIZE)
      ? 0 : serialconsole_rx_buffer_head + 1;

  if(new_head != serialconsole_rx_buffer_tail) {
    serialconsole_rx_buffer[serialconsole_rx_buffer_head] = data;
    serialconsole_rx_buffer_head = new_head;
  } /* else drop it */
}
