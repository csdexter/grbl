/*
  coolant_control.h - coolant control methods
  Part of Grbl

  Copyright (c) 2012 Sungeun K. Jeon

  Grbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Grbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Grbl. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef coolant_control_h
#define coolant_control_h

#include <stdint.h>

#define COOLANT_MIST 0x02
#define COOLANT_FLOOD 0x01
#define COOLANT_OFF 0x00

void coolant_init(void);
void coolant_stop(void);
void coolant_run(uint8_t mode);

#endif
