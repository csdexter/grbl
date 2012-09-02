/*
 * config-avr.h - AVR-specific compile-time configuration
 *
 *  Created on: Aug 31, 2012
 *      Author: csdexter
 */

#ifndef CONFIG_H
# error This file must not be included directly, use config.h instead!
#endif
#ifdef CONFIG_AVR_H
# error The configuration header of another HAL has already been included!
#else
#define CONFIG_AVR_H

/* PIN ASSIGNMENTS begin ----------- */
// Define pin-assignments for stepper movement (by default they are taken to be
// active-high, edge [i.e. the motors will move one step on the rising edge];
// see settings.c for how to invert them if your hardware setup requires it as
// well as how to control the minimum pulse width to match what your controller
// can take)
#define STEPPING_PORT      PORTC
#define STEPPING_PIN       PINC
#define X_STEP_BIT         0  // Uno Analog Pin 0
#define Y_STEP_BIT         1  // Uno Analog Pin 1
#define Z_STEP_BIT         2  // Uno Analog Pin 2
#if(0)
#define STEPPING_PORT HOST_GPIO_PC
#endif

// Define pin-assignments for stepper direction (by default they are taken to be
// true logic, level [i.e. 0 is CW and causes the axis to move towards zero and
// 1 is CCW and causes the axis to move away from zero -- according to the
// standard axis directions used in machining]; see settings.c for how to invert
// them if your hardware setup requires it and STEP_PULSE_DELAY below for
// matching your controller's time constraints)
#define DIRECTION_PORT     PORTC
#define X_DIRECTION_BIT    3  // Uno Analog Pin 3
#define Y_DIRECTION_BIT    4  // Uno Analog Pin 4
#define Z_DIRECTION_BIT    5  // Uno Analog Pin 5
#if(0)
#define DIRECTION_PORT HOST_GPIO_PC
#endif

// Uncomment the next line to enable support for a Stepper Power On/Off control
// line, for drivers that have/use one.
// Please note that disabling this will also disable STEPPER_IDLE_LOCK_TIME
// below since we cannot lock the steppers if we have no control over their
// active status
//#define STEPPERS_DISABLE
#define STEPPERS_DISABLE_PORT   PORTB
#define STEPPERS_DISABLE_BIT    0  // Uno Digital Pin 8
#if(0)
#define STEPPERS_DISABLE HOST_GPIO_PB0
#endif

// Define pin-assignments for endstop switches (by default they are taken to be
// active-low, level [i.e. they transition from 1 to 0 when the machine hits the
// limit and stay 0 while the machine is in the limit area]; see settings.c for
// how to invert them if your hardware setup requires it)
// Please see below for topology support (i.e. where are the switches installed
// on your machine) and hard/soft limits
#define LIMIT_PIN     PIND
#define X_LIMIT_BIT   2  // Uno Digital Pin 2
#define Y_LIMIT_BIT   3  // Uno Digital Pin 3
#define Z_LIMIT_BIT   4  // Uno Digital Pin 4
#if(0)
#define LIMIT_PORT HOST_GPIO_PD
#endif

// Define pin-assignments for spindle control (this is always active-high, level
// [i.e. 1 causes the spindle to start and run until 0 is output])
#define SPINDLE_ENABLE HOST_GPIO_PD5 // Uno Digital Pin 5

// Uncomment the next line to enable support for a Charge Pump/Watchdog signal
// line. This will generate a 12.5kHz 50% duty cycle square wave signal on D6
// which can then be connected to the Charge Pump/Watchdog input of your servo
// controller. The pin is hardcoded because the signal is generated in hardware
// using Timer 0, Channel A which is, in turn, tied to PD6 (which corresponds
// to Digital 6 on the Arduino).
// TODO: switch back to configurable when all ifdefs are in place for the function generator macro
#define CHARGE_PUMP HOST_GPIO_PD6 // Uno Digital Pin 6, for reference only, not configurable

// Uncomment the next line to enable support for a Spindle Direction (CW or CCW)
// control line, for drivers that have/use one (this is always true logic, level
// [i.e. 0 is CW and 1 is CCW])
//#define SPINDLE_DIRECTION HOST_GPIO_PD7 // Uno Digital Pin 7

// IMPORTANT: if you made any changes above, make sure to update the following
// #defines as well
#define PORTC_DIRECTIONS _BV(X_STEP_BIT) | _BV(Y_STEP_BIT) | \
  _BV(Z_STEP_BIT) | _BV(X_DIRECTION_BIT) | _BV(Y_DIRECTION_BIT) | \
  _BV(Z_DIRECTION_BIT)
#define PORTD_DIRECTIONS _BV(host_bit_of_output(SPINDLE_ENABLE)) | \
  _BV(host_bit_of_output(CHARGE_PUMP))
#define PORTB_DIRECTIONS _BV(STEPPERS_DISABLE_BIT)

// Useful bit for the above
#define SETUP_IO() {\
  host_gpio_direction(HOST_GPIO_PB, PORTB_DIRECTIONS, HOST_GPIO_MODE_BYTE); \
  host_gpio_direction(HOST_GPIO_PC, PORTC_DIRECTIONS, HOST_GPIO_MODE_BYTE); \
  host_gpio_direction(HOST_GPIO_PD, PORTD_DIRECTIONS, HOST_GPIO_MODE_BYTE);}
/* PIN ASSIGNMENTS end ------------- */


#endif /* CONFIG_AVR_H */
