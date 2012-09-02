/*
  config.h - compile time configuration
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

#ifndef CONFIG_H
#define CONFIG_H

#include "host.h"

#if defined(__GNUC__)
# if defined(__AVR__)
#  include "config-avr.h"
# elif defined(__arm__)
#  include "config-arm.h"
# elif defined(__i386__)
#  include "config-i386.h"
# endif
#endif


// Machine limits topology options
// Define LIMIT_HARD if you want to be able to sense hardware endstops and use
// them for calibrating the machine. Define LIMIT_SOFT if you want to keep track
// of the machine's position in software and assume the limit is hit if the
// machine reaches a certain coordinate.
// All _VALUEs are in mm and represent maximum machine travel as seen from
// G-code [e.g. if you declare a 100mm axis as [-50, 50] and then try to move to
// 55, you'll get an error since the code will think you're trying to go beyond
// the limits].
// Please note the current implementation supports at most 3 limit switches, at
// most one per axis.
// Please note that configuring for no endstops on any axis means the code will
// have no means to recalibrate that axis.
#define LIMIT_HARD // Support for hardware endswitches
#define LIMIT_SOFT // Support for software enforced limits
#define LIMIT_TYPE_SOFT 0
#define LIMIT_TYPE_HARD 1
#define X_LIMIT_NEG_TYPE LIMIT_TYPE_SOFT
#define X_LIMIT_NEG_VALUE 0.0
#define X_LIMIT_POS_TYPE LIMIT_TYPE_HARD
#define X_LIMIT_POS_VALUE 720.0
#define Y_LIMIT_NEG_TYPE LIMIT_TYPE_SOFT
#define Y_LIMIT_NEG_VALUE 0.0
#define Y_LIMIT_POS_TYPE LIMIT_TYPE_HARD
#define Y_LIMIT_POS_VALUE 420.0
#define Z_LIMIT_NEG_TYPE LIMIT_TYPE_SOFT
#define Z_LIMIT_NEG_VALUE 0.0
#define Z_LIMIT_POS_TYPE LIMIT_TYPE_HARD
#define Z_LIMIT_POS_VALUE 110.0

// Specifies the number of work coordinate systems grbl will support (G54 - G59).
// This parameter must be one or greater, currently supporting up to a value of 6.
#define N_COORDINATE_SYSTEM 1

// This parameter sets the delay time before disabling the steppers after the final block of movement.
// A short delay ensures the steppers come to a complete stop and the residual inertial force in the 
// CNC axes don't cause the axes to drift off position. This is particularly important when manually 
// entering g-code into grbl, i.e. locating part zero or simple manual machining. If the axes drift,
// grbl has no way to know this has happened, since stepper motors are open-loop control. Depending
// on the machine, this parameter may need to be larger or smaller than the default time.
// NOTE: If set to zero, the delay will not be compiled.
// NOTE: If the current hardware configuration doesn't have a means to switch stepper motor power
//       on and off (i.e. STEPPER_DISABLE is NOT #defined), the delay is also not compiled.
#define STEPPER_IDLE_LOCK_TIME 25 // (milliseconds) - Integer > 0

// The temporal resolution of the acceleration management subsystem. Higher number give smoother
// acceleration but may impact performance.
// NOTE: Increasing this parameter will help any resolution related issues, especially with machines 
// requiring very high accelerations and/or very fast feedrates. In general, this will reduce the 
// error between how the planner plans the motions and how the stepper program actually performs them.
// However, at some point, the resolution can be high enough, where the errors related to numerical 
// round-off can be great enough to cause problems and/or it's too fast for the Arduino. The correct
// value for this parameter is machine dependent, so it's advised to set this only as high as needed.
// Approximate successful values can range from 30L to 100L or more.
#define ACCELERATION_TICKS_PER_SECOND 50L

// Minimum planner junction speed. Sets the default minimum speed the planner plans for at the end
// of the buffer and all stops. This should not be much greater than zero and should only be changed
// if unwanted behavior is observed on a user's machine when running at very slow speeds.
#define MINIMUM_PLANNER_SPEED 0.0 // (mm/min)

// Minimum stepper rate. Sets the absolute minimum stepper rate in the stepper program and never runs
// slower than this value, except when sleeping. This parameter overrides the minimum planner speed.
// This is primarily used to guarantee that the end of a movement is always reached and not stop to
// never reach its target. This parameter should always be greater than zero.
#define MINIMUM_STEPS_PER_MINUTE 800 // (steps/min) - Integer value only

// Number of arc generation iterations by small angle approximation before exact arc trajectory 
// correction. This parameter maybe decreased if there are issues with the accuracy of the arc
// generations. In general, the default value is more than enough for the intended CNC applications
// of grbl, and should be on the order or greater than the size of the buffer to help with the 
// computational efficiency of generating arcs.
#define N_ARC_CORRECTION 25 // Integer (1-255)

// Time delay increments performed during a dwell. The default value is set at 50ms, which provides
// a maximum time delay of roughly 55 minutes, more than enough for most any application. Increasing
// this delay will increase the maximum dwell time linearly, but also reduces the responsiveness of 
// run-time command executions, like status reports, since these are performed between each dwell 
// time step. Also, keep in mind that the Arduino delay timer is not very accurate for long delays.
#define DWELL_TIME_STEP 50 // Integer (1-255) (milliseconds)

// ---------------------------------------------------------------------------------------
// FOR ADVANCED USERS ONLY: 

// Toggles XON/XOFF software flow control for serial communications. Not officially supported
// due to problems involving the Atmega8U2 USB-to-serial chips on current Arduinos. The firmware
// on these chips do not support XON/XOFF flow control characters and the intermediate buffer 
// in the chips cause latency and overflow problems with standard terminal programs. However, 
// using specifically-programmed UI's to manage this latency problem has been confirmed to work.
// As well as, older FTDI FT232RL-based Arduinos(Duemilanove) are known to work with standard
// terminal programs since their firmware correctly manage these XON/XOFF characters. In any
// case, please report any successes to grbl administrators!
#define ENABLE_XONXOFF 0 // Boolean. Default disabled.

// Creates a delay between the direction pin setting and corresponding step pulse by creating
// another interrupt (Timer2 compare) to manage it. The main Grbl interrupt (Timer1 compare) 
// sets the direction pins, and does not immediately set the stepper pins, as it would in 
// normal operation. The Timer2 compare fires next to set the stepper pins after the step 
// pulse delay time, and Timer2 overflow will complete the step pulse, except now delayed 
// by the step pulse time plus the step pulse delay. (Thanks langwadt for the idea!)
//   This is an experimental feature that should only be used if your setup requires a longer
// delay between direction and step pin settings (some opto coupler based drivers), as it may
// adversely effect Grbl's high-end performance (>10kHz). Please notify Grbl administrators 
// of your successes or difficulties, as we will monitor this and possibly integrate this as a 
// standard feature for future releases. However, we suggest to first try our direction delay
// hack/solution posted in the Wiki involving inverting the stepper pin mask.
// NOTE: If set greater than zero, step pulse delay will be compiled and enabled. Also, the 
// total delay added with the Grbl settings pulse microseconds must not exceed 127 ms.
#define STEP_PULSE_DELAY 0 // Step pulse delay in microseconds. Default disabled.

// ---------------------------------------------------------------------------------------

// TODO: The following options are set as compile-time options for now, until the next EEPROM 
// settings version has solidified. 
#define CYCLE_AUTO_START 1    // Cycle auto-start boolean flag for the planner.
#define BLOCK_DELETE_ENABLE 0 // Block delete enable/disable flag during g-code parsing
#define REPORT_INCH_MODE 0    // Status reporting unit mode (1 = inch, 0 = mm)
#if REPORT_INCH_MODE
  #define DECIMAL_PLACES 3
  #define DECIMAL_MULTIPLIER 1000 // 10^DECIMAL_PLACES
#else
  #define DECIMAL_PLACES 2  // mm-mode
  #define DECIMAL_MULTIPLIER 100
#endif


#endif
