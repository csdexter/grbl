/*
  interrupt.c - replacement for the avr library of the same name to provide
  dummy register variables

  Part of Grbl Simulator

  Copyright (c) 2012 Jens Geisler

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

#include "interrupt.h"

// dummy register variables
uint8_t timsk1;
uint8_t timsk2;
uint8_t ocr1a;
uint8_t tcnt2;
uint8_t tccr2b;
uint8_t tccr1b;
uint8_t tccr1a;
uint8_t tccr2a;

void sei() {};
