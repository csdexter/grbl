/*
  nuts_bolts.c - Shared functions
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

#include <stdbool.h>
#include <stdlib.h>

#include "config.h"


// Resolution of IEEE754 single (ANSI C float) is exactly 7.22 decimal digits.
// Current day CNCs have micron resolution and under 10m carriages thus giving
// a range of 0.001mm to 9999.999mm (or 0.0001" to 999.9999"), fully covered by
// the available precision and resolution of float.
// Extra high precision CNCs are usually small so they are also covered with a
// range of 0.0001mm to 999.9999mm (or 0.00001" to 99.99999").
// NOTE: if you enable scaling and set it to a negative enough factor, you may
//       indeed exhaust available precision. We intentionally do not take any
//       countermeasures for that, encouraging users to make wise use of the
//       available numeric range instead.
// We only set the allowable number of digits to 8 here for rounding purposes.
#define MAX_INT_DIGITS 8

// Extracts a floating point value from a string. The following code is based
// loosely on the avr-libc strtod() function by Michael Stumpf and
// Dmitry Xmelkov and many freely available conversion method examples, but has
// been highly optimized for Grbl. For known CNC applications (see above), the
// typical number of decimals (significant figures to the right of the decimal
// point) is always less than or equal to four.
// Scientific notation is officially not supported by G-Code and the 'E'
// character is a G-Code word on some CNC systems (threading feed on lathes and
// extruder speed on RepRap, to name a few) -- therefore, 'E' notation will not
// be recognized.
// NOTE: Thanks to Radu-Eosif Mihailescu for identifying the issues with using
//       strtod().
bool read_float(char *line, uint8_t *char_counter, float *float_ptr) {
  char *ptr = line + *char_counter;
  unsigned char c;
  bool isNegative = false, seenDecimalPoint = false;
  uint32_t intval = 0;
  int8_t exp = 0;
  uint8_t nDigits = 0;
  float result;

  // Test for leading sign character. All spaces have already been
  // stripped from the line.
  if(*ptr == '-') isNegative = true;
  if(*ptr == '+' || *ptr == '-') c = *(++ptr);
  else c = *ptr;

  // Extract number into fast integer. Track decimal point position in terms of
  // exponent value.
  while((c <= '9' && c >= '0') || (c == '.' && !seenDecimalPoint)) {
    if(c == '.') seenDecimalPoint = true;
    else {
      nDigits++;
      if(nDigits <= MAX_INT_DIGITS) {
        if(seenDecimalPoint) exp--;
        intval = (intval << 3) + (intval << 1) + (c - '0'); // intval * 10 + c
      } else if(!(seenDecimalPoint)) exp++; // Drop overflow digits
    }
    c = *(++ptr);
  }

  // Return now if no digits have been read.
  if(!nDigits) return false;

  // Convert integer into floating point.
  result = intval;

  // Apply decimal point. Should perform no more than two floating point
  // multiplications for the expected range of E0 to E-4.
  if(result) {
    while(exp <= -2) {
      result *= 0.01;
      exp += 2;
    }
    if(exp < 0) result *= 0.1;
    else if(exp > 0) do result *= 10.0; while (--exp > 0);
  }

  // Apply sign.
  if(isNegative) *float_ptr = -result;
  else *float_ptr = result;

  *char_counter = ptr - line; // Set char_counter to next statement

  return true;
}
