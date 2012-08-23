/*
 * host-avr.h - HAL for Atmel AVR family (8 bit)
 *
 *  Created on: Aug 21, 2012
 *      Author: csdexter
 */

#ifndef HOST_H_
# error This file must not be included directly, use host.h instead!
#endif
#ifdef HOST_HAL_H_
# error Another HAL has already been included!
#else

/* Host-specific includes */
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/crc16.h>


/* Host-specific interrupt enable */
#define host_sei() sei()

/* Host-specific delays */
void _avr_delay_helper_ms(uint16_t ms);
void _avr_delay_helper_us(uint16_t us);
#define host_delay_ms(ms) _avr_delay_helper_ms(ms)
#define host_delay_us(us) _avr_delay_helper_us(us)

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


#endif /* HOST_HAL_H_ */
