/*
 * config-avr.h - AVR-specific compile-time configuration
 *
 *  Created on: Aug 31, 2012
 *      Author: csdexter
 *
 * NOTE: despite this file containing architecture-specific *information*, an
 *       effort is made to keep it free from architecture-specific *code* so
 *       that it remains as portable as the core.
 */

#ifndef CONFIG_H
# error This file must not be included directly, use config.h instead!
#endif
#ifdef CONFIG_AVR_H
# error The configuration header of another HAL has already been included!
#else
#define CONFIG_AVR_H

// Define pin-assignments for stepper movement (by default they are taken to be
// active-high, edge [i.e. the motors will move one step on the rising edge];
// see settings.c for how to invert them if your hardware setup requires it as
// well as how to control the minimum pulse width to match what your controller
// can take)
#define STEP_X HOST_GPIO_PC0
#define STEP_Y HOST_GPIO_PC1
#define STEP_Z HOST_GPIO_PC2

// Define pin-assignments for stepper direction (by default they are taken to be
// true logic, level [i.e. 0 causes the axis to move away from zero and 1 causes
// the axis to move towards zero -- according to ISO841-2001]; see settings.c
// for how to invert them if your hardware setup requires it and
// STEP_PULSE_DELAY in config.h for matching your controller's time constraints)
//TODO: move the whole invert stuff out of the core and into the actual
//      host_gpio_write() call to keep hardware specifics out of the core logic
#define DIR_X HOST_GPIO_PC3
#define DIR_Y HOST_GPIO_PC4
#define DIR_Z HOST_GPIO_PC5

// Define pin-assignments for limit switches (by default they are taken to be
// active-low, level [i.e. they transition from 1 to 0 when the machine hits the
// limit and stay 0 while the machine is in the limit area]; see settings.c for
// how to invert them if your hardware setup requires it)
// Please see below for topology support (i.e. where are the switches installed
// on your machine) and hard/soft limits
#define LIMIT_X HOST_GPIO_PD2
#define LIMIT_Y HOST_GPIO_PD3
#define LIMIT_Z HOST_GPIO_PD4

// Uncomment the next line to enable support for a Stepper Disable control
// line, for drivers that have/use one (this is always true logic, level [i.e.
// 0 is Power On and 1 is Power Off]).
// NOTE: disabling this will also disable STEPPER_IDLE_LOCK_TIME below since we
// cannot lock the steppers if we have no control over their active status.
//TODO: whenever we will support axis brakes/clamps, update the above notice
// NOTE: on some stepper drivers/controllers, the Enable/Disable line may reset
// state of the driver chips which means the motors will advance to the next
// full step the next time they're enabled (i.e. microstepping state is lost)
// -- you may not want to use it if that is the case.
//#define STEPPERS_DISABLE HOST_GPIO_PB0

// Define pin-assignments for spindle control (this is always active-high, level
// [i.e. 1 causes the spindle to start and run until 0 is output])
#define SPINDLE_ENABLE HOST_GPIO_PD5 // Uno Digital Pin 5

// Uncomment the next line to enable support for a Spindle Direction (CW or CCW)
// control line, for drivers that have/use one (this is always true logic, level
// [i.e. 0 is CW and 1 is CCW])
//#define SPINDLE_DIRECTION HOST_GPIO_PD7 // Uno Digital Pin 7

// Uncomment the next line to enable support for a Charge Pump/Watchdog signal
// line. This will generate a 12.5kHz 50% duty cycle square wave signal on D6
// which can then be connected to the Charge Pump/Watchdog input of your servo
// controller. The pin is hardcoded because the signal is generated in hardware
// using Timer 0, Channel A which is, in turn, tied to PD6 (which corresponds
// to Digital 6 on the Arduino).
// NOTE: the rest of grbl uses Timers 1 and 2 so the only other option is
// OC0B on PD5 (Arduino Digital 5).
#define CHARGE_PUMP HOST_GPIO_PD6 // Uno Digital Pin 6


#endif /* CONFIG_AVR_H */
