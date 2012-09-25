/*
 * config-i386.h - hosting-specific compile-time configuration
 *
 *  Created on: Sep 25, 2012
 *      Author: csdexter
 *
 * NOTE: despite this file containing architecture-specific *information*, an
 *       effort is made to keep it free from architecture-specific *code* so
 *       that it remains as portable as the core.
 * NOTE: we're a debugging/development HAL so we'll tend to support
 *       "everything", hence no "uncomment next line"s here.
 */

#ifndef CONFIG_H
# error This file must not be included directly, use config.h instead!
#endif
#ifdef CONFIG_HAL_H
# error The configuration header of another HAL has already been included!
#else
#define CONFIG_HAL_H

// See the AVR HAL for detailed explanations for each symbolic constant
#define STEP_X HOST_GPIO_STEP_X
#define STEP_Y HOST_GPIO_STEP_Y
#define STEP_Z HOST_GPIO_STEP_Z
#define DIR_X HOST_GPIO_DIR_X
#define DIR_Y HOST_GPIO_DIR_Y
#define DIR_Z HOST_GPIO_DIR_Z
#define LIMIT_X HOST_GPIO_LIMIT_X
#define LIMIT_Y HOST_GPIO_LIMIT_Y
#define LIMIT_Z HOST_GPIO_LIMIT_Z
#define STEPPERS_DISABLE HOST_GPIO_SERVO_OFF
#define SPINDLE_ENABLE HOST_GPIO_SPINDLE_ON
#define SPINDLE_DIRECTION HOST_GPIO_SPINDLE_CCW
#define CHARGE_PUMP HOST_GPIO_CHARGE_PUMP
#define CFLOOD_ENABLE HOST_GPIO_COOL_FLOOD
#define CSPRAY_ENABLE HOST_GPIO_COOL_MIST


#endif /* CONFIG_HAL_H */
