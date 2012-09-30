/*
 * host-i386.h - HAL for hosting (compiling and running grbl on any GNU-C
 *               capable machine)
 *
 *  Created on: Sep 25, 2012
 *      Author: csdexter
 *
 * NOTE: for historical reasons, the host HAL is called "i386" within this
 *       source tree. A goodwill effort has been made to keep the HAL code as
 *       ANSI C and as portable as possible, thus enabling this HAL to be used
 *       on any architecture and any machine that has a C compiler and supports
 *       the common-sense POSIX-isms (like file I/O etc.)
 *       The rest of the code (grbl itself) has already been ported to HAL so,
 *       at least in theory (and is the intent of yours truly), you should be
 *       able to compile and run grbl + hosting HAL pretty much anywhere.
 * NOTE: the original usage intent for this HAL was debugging of existing grbl
 *       code and iterative testing and refining of new features before
 *       attempting to run them on the actual MCU hardware. As such, it consists
 *       mainly of mockups and debug print statements tailored to give you an
 *       insight into how grbl operates. It was not intended for running grbl
 *       on the host for production tasks (i.e. driving an actual CNC), at the
 *       very least because the host OS is not real-time.
 */

#ifndef HOST_H_
# error This file must not be included directly, use host.h instead!
#endif
#ifdef HOST_HAL_H_
# error Another HAL has already been included!
#else
#define HOST_HAL_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Host-specific includes */
/* #include dummy */


/* Host-specific opaque initialization */
void host_init(int argc, char **argv); // Set environment up when hosting

/* Host-specific interrupt enable */
void host_sei(void);
/* Host-specific interrupt vector declaration */
#define HOST_INTERRUPT(x) void x(void);\
  void x(void)
/* Host-specific interrupt vector registration */
#define HOST_INTERRUPT_FAMILY_TIMER 0x01
#define _i386_stringify(x) #x
void i386_register_interrupt(const char *name, void(*isr)(void));
#define host_register_interrupt(vector) i386_register_interrupt(_i386_stringify(vector), vector)

/* Host-specific delays */
void i386_delay_us(uint32_t us);
#define host_delay_ms(ms) i386_delay_us(ms * 1000UL)
#define host_delay_us(us) i386_delay_us(us)

/* Host-specific constant string treatment (literal) */
#define _S(s) s
/* ... (type) */
#define _S_t(s) s
/* ... (access method) */
#define host_fetch_S(s) *(s)

/* Host-specific NVS methods */
void host_nvs_store_byte(uint8_t *address, uint8_t value);
void host_nvs_store_word(uint16_t *address, uint16_t value);
void host_nvs_store_data(uint8_t *address, const void *value, size_t size);
uint8_t host_nvs_fetch_byte(uint8_t *address);
uint16_t host_nvs_fetch_word(uint16_t *address);
void host_nvs_fetch_data(uint8_t *address, void *buffer, size_t size);

/* Host-specific CRC methods */
uint8_t host_crc8(uint8_t crc, uint8_t data);
uint16_t host_crc16(uint16_t crc, uint8_t data);

/* Serial console interface */
void host_serialconsole_init();
void host_serialconsole_reset();
/* Returns CONSOLE_NO_DATA if nothing to read */
char host_serialconsole_read(void);
bool host_serialconsole_write(char c, bool block);
bool host_serialconsole_printinteger(uint32_t n, bool block);
bool host_serialconsole_printbinary(uint8_t n, bool block);
bool host_serialconsole_printfloat(float n, uint8_t precision, bool block);

/* Host GPIO interface */
#define HOST_GPIO_STEP_X 0x01
#define HOST_GPIO_STEP_Y 0x02
#define HOST_GPIO_STEP_Z 0x03
#define HOST_GPIO_DIR_X 0x04
#define HOST_GPIO_DIR_Y 0x05
#define HOST_GPIO_DIR_Z 0x06
#define HOST_GPIO_LIMIT_X 0x07
#define HOST_GPIO_LIMIT_Y 0x08
#define HOST_GPIO_LIMIT_Z 0x09
#define HOST_GPIO_SERVO_OFF 0x0A
#define HOST_GPIO_SPINDLE_ON 0x0B
#define HOST_GPIO_SPINDLE_CCW 0x0C
#define HOST_GPIO_CHARGE_PUMP 0x0D
#define HOST_GPIO_COOL_FLOOD 0x0E
#define HOST_GPIO_COOL_MIST 0x0F
void host_gpio_direction(uint8_t output, bool direction, bool mode);
uint8_t host_gpio_read(uint8_t output, bool mode);
void host_gpio_write(uint8_t output, uint8_t value, bool mode);
void host_gpio_toggle(uint8_t output, bool mode);

/* Host Timer interface */
#define HOST_TIMER_FOSC 16000000UL
#define HOST_TIMER_CHANNEL_A 0x02
#define HOST_TIMER_CHANNEL_B 0x03
#define HOST_TIMER_INTERRUPT_OVERFLOW_flag 0x01
#define HOST_TIMER_INTERRUPT_OVERFLOW_vector O
#define HOST_TIMER_INTERRUPT_COMPARE_A_flag HOST_TIMER_CHANNEL_A
#define HOST_TIMER_INTERRUPT_COMPARE_A_vector A
#define HOST_TIMER_INTERRUPT_COMPARE_B_flag HOST_TIMER_CHANNEL_B
#define HOST_TIMER_INTERRUPT_COMPARE_B_vector B
#define HOST_TIMER_PRESCALER_COUNT_0 5
#define HOST_TIMER_PRESCALERS_0 {1, 8, 64, 256, 1024}
#define HOST_TIMER_PRESCALER_COUNT_1 5
#define HOST_TIMER_PRESCALERS_1 {1, 8, 64, 256, 1024}
#define HOST_TIMER_PRESCALER_COUNT_2 7
#define HOST_TIMER_PRESCALERS_2 {1, 8, 32, 64, 128, 256, 1024}
#define HOST_TIMER_COMPARE_MAX_0 0x100UL
#define HOST_TIMER_COMPARE_MAX_1 0x10000UL
#define HOST_TIMER_COMPARE_MAX_2 0x100UL
#define ___i386_timer_vector_name(timer,which) T ## timer ## _ ## which ## _V
#define __i386_timer_vector_name(timer,which) ___i386_timer_vector_name(timer,which)
#define ___i386_timer_vector_type(which) which ## _vector
#define __i386_timer_vector_type(which) ___i386_timer_vector_type(which)
#define host_timer_vector_name(timer,which) __i386_timer_vector_name(timer,__i386_timer_vector_type(which))
void i386_timer_enable_interrupt(uint8_t timer, uint8_t which);
void i386_timer_disable_interrupt(uint8_t timer, uint8_t which);
#define ___i386_if_of_timer(which) which ## _flag
#define __i386_if_of_timer(which) ___i386_if_of_timer(which)
#define _host_timer_enable_interrupt(timer,which) i386_timer_enable_interrupt(timer, which)
#define host_timer_enable_interrupt(timer,which) _host_timer_enable_interrupt(timer,__i386_if_of_timer(which))
#define _host_timer_disable_interrupt(timer,which) i386_timer_disable_interrupt(timer, which)
#define host_timer_disable_interrupt(timer,which) _host_timer_disable_interrupt(timer,__i386_if_of_timer(which))
void host_timer_set_compare(uint8_t timer, uint8_t channel, uint32_t value);
#define host_prescaler_of_divisor(timer,divisor) (divisor)
void host_timer_set_count(uint8_t timer, uint32_t count);
void host_timer_set_prescaler(uint8_t timer, uint8_t prescaler);
void host_timer_enable_ctc(uint8_t timer);
uint32_t i386_timer_set_reload(uint8_t timer, uint32_t cycles);
#define host_timer_set_reload(timer,cycles,actual_cycles) \
  actual_cycles = i386_timer_set_reload(timer, cycles)

/* Host waveform generator interface */
void host_functiongenerator_start(uint8_t output, uint32_t frequency, uint8_t form);
void host_functiongenerator_stop(uint8_t output);


#endif /* HOST_HAL_H_ */
