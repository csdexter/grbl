/*
  print.c - Functions for formatting output strings
  Part of Grbl

  Copyright (c) 2009-2011 Simen Svale Skogsrud
  Copyright (c) 2011-2012 Sungeun K. Jeon

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

/* This code was initially inspired by the wiring_serial module by
 * David A. Mellis which used to be a part of the Arduino project. */

#include <math.h>
#include <stdbool.h>

#include "config.h"


void printString(const char *s) {
  while (*s) host_serialconsole_write(*s++, true);
}

/* Print a constant string (a message). Depending on architecture and
 * configuration, this may mean said string is coming from a different address
 * space or via a possibly totally different access method than a usual char *.
 * The only specification is that the passed pointer can be transformed into an
 * usual string on a character-by-character basis by calling a HAL function
 * that will fetch it for us.
 */
void printMessage(const char *s) {
  char c;

  while ((c = host_fetch_S(s++))) host_serialconsole_write(c, true);
}

void printBinary(uint8_t n) {
  uint8_t i;

  for(i = 8; i; i--) {
    host_serialconsole_write('0' + (n & 0x80), true);
    n <<= 1;
  }
}

/* Math is already part of this code, sprintf() isn't. Peruse math, then */
void printInteger(uint32_t n) {
  uint32_t magnitude;

  if(n < 10) host_serialconsole_write('0' + n, true);
  else {
    magnitude = powf(10, floorf(log10f(n)));
    printInteger(n / magnitude);
    printInteger(n % magnitude);
  }
}

/* Math is already part of this code, sprintf() isn't. Peruse math, then */
void printFloat(float n) {
  float integer, decimals;

  if(signbit(n)) {
    host_serialconsole_write('-', true);
    n = -n;
  }

  decimals = modff(n, &integer);
  
  printInteger((uint32_t)integer);
  host_serialconsole_write('.', true);
  printInteger(lroundf(decimals * DECIMAL_MULTIPLIER));
}
