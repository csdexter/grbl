/*
  print.h - Functions for formatting output strings
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

/* This code was initially inspired by the wiring_serial module by David A. Mellis which
   used to be a part of the Arduino project. */ 

#ifndef print_h
#define print_h

#include <stdint.h>


/* Print an unsigned integer value to the system console in base 10 */
void printInteger(uint32_t n);

/* Print a byte to the system console in binary */
void printBinary(uint8_t n);

/* Print a floating point value to the system console in fixed precision format,
 * as dictated by settings in config.h
 */
void printFloat(float n);


#endif
