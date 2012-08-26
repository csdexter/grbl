/*
 * host-avr.h - HAL for Atmel AVR family (8 bit)
 *
 *  Created on: Aug 21, 2012
 *      Author: csdexter
 *
 * NOTE: in accordance with <util/setbaud.h> API, BAUD needs to be #defined
 *       before we are included
 */

#ifndef HOST_H_
# error This file must not be included directly, use host.h instead!
#endif
#ifdef HOST_HAL_H_
# error Another HAL has already been included!
#else

#include <stdbool.h>
#include <stdint.h>

/* Host-specific includes */
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/crc16.h>


/* Host-specific interrupt enable */
#define host_sei() sei()

/* Host-specific delays */
void host_delay_ms(uint16_t ms);
void host_delay_us(uint16_t us);

/* Host-specific constant string treatment (literal) */
#define _S(s) PSTR(s)
/* ... (type) */
#define _S_t(s) s PROGMEM
/* ... (access method) */
#define host_fetch_S(s) pgm_read_byte(s)

/* Host-specific NVS methods */
#define host_nvs_store_byte(a, v) eeprom_update_byte(a, v)
#define host_nvs_store_word(a, v) eeprom_update_word(a, v)
#define host_nvs_store_data(a, v, s) eeprom_update_block(v, a, s)
#define host_nvs_fetch_byte(a) eeprom_read_byte(a)
#define host_nvs_fetch_word(a) eeprom_read_word(a)
#define host_nvs_fetch_data(a, t, s) eeprom_read_block(t, a, s)

/* Host-specific CRC methods */
#define host_crc8(crc, data) _crc_ibutton_update(crc, data)
#define host_crc16(crc, data) _crc16_update(crc, data)

/* Serial console interface */
void host_serialconsole_init();
void host_serialconsole_reset();
/* Returns CONSOLE_NO_DATA if nothing to read */
char host_serialconsole_read(void);
/* Blocks until buffer space is available if block is set to true, returns
 * false if in non-blocking mode and no buffer space */
bool host_serialconsole_write(char c, bool block);
/* Host serial console unsigned integer output in base 10. Blocks until buffer
 * space is available if block is set to true, returns false if in non-blocking
 * mode and no buffer space */
bool host_serialconsole_printinteger(uint32_t n, bool block);
/* Host serial console byte output in base 2. Blocks until buffer space is
 * available if block is set to true, returns false if in non-blocking mode and
 * no buffer space */
bool host_serialconsole_printbinary(uint8_t n, bool block);
/* Host serial console float output with precision figures after the decimal
 * point. Blocks until buffer space is available if block is set to true,
 * returns false if in non-blocking mode and no buffer space */
bool host_serialconsole_printfloat(float n, uint8_t precision, bool block);


#endif /* HOST_HAL_H_ */
