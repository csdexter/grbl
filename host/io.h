/*
  io.h - host architecture counterparts of avr/io.h contents
  Part of Grbl

  Copyright (c) 2012 Radu - Eosif Mihailescu

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

#ifndef _HOST_IO_H_
#define _HOST_IO_H_

#include <stdint.h>

/* Originally from avr/sfr_defs.h via avr/io.h for AVR */
#define _BV(bit) (1 << (bit))

/* Register mock-ups */
typedef struct {
	uint8_t ddr[3];
} TAVRIORegisters;

extern TAVRIORegisters avrIORegisters;

#define DDRB avrIORegisters.ddr[0]
#define DDRC avrIORegisters.ddr[1]
#define DDRD avrIORegisters.ddr[2]

#endif
