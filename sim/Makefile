#  Part of Grbl
#
#  Copyright (c) 2012 Jens Geisler
#
#  Grbl is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  Grbl is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with Grbl.  If not, see <http://www.gnu.org/licenses/>.

OBJECTS    = simulator.o ../planner.o ../settings.o ../print.o ../nuts_bolts.o eeprom.o serial.o avr\pgmspace.o avr\interrupt.o util\delay.o ../stepper.o ../gcode.o ../spindle_control.o ../motion_control.o ../limits.o simstepper.o ../protocol.o runtime.o

             
CLOCK      = 16000000
EXE_NAME= grbl_sim.exe
COMPILER= C:\MinGW\bin\gcc.exe
COMPILE = $(COMPILER) -Wall -Os -DF_CPU=$(CLOCK) -include config.h -I.

# symbolic targets:
all:	main

new: clean main

.c.o:
	$(COMPILE) -c $< -o $@ 

.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@
# "-x assembler-with-cpp" should not be necessary since this is the default
# file type for the .S (with capital S) extension. However, upper case
# characters are not always preserved on Windows. To ensure WinAVR
# compatibility define the file type manually.

.c.s:
	$(COMPILE) -S $< -o $@

clean:
	rm -f $(EXE_NAME) $(OBJECTS)

# file targets:
main: $(OBJECTS)
	$(COMPILE) -o $(EXE_NAME) $(OBJECTS) -lm -Wl,--gc-sections

