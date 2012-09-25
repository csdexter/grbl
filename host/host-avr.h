/*
 * host-avr.h - HAL for Atmel AVR family (8 bit)
 *
 *  Created on: Aug 21, 2012
 *      Author: csdexter
 *
 * NOTE: in accordance with <util/setbaud.h> API, BAUD needs to be #defined
 *       before we are included
 * NOTE: this file is tuned for the ATmega328p that you can usually find
 *       powering the Arduinos. If another 8-bit AVR is targeted, conditional
 *       defines should be inserted to make the code portable among sub-
 *       architectures.
 */

#ifndef HOST_H_
# error This file must not be included directly, use host.h instead!
#endif
#ifdef HOST_HAL_H_
# error Another HAL has already been included!
#else
#define HOST_HAL_H_

#include <stdbool.h>
#include <stdint.h>

/* Host-specific includes */
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/version.h>
#include <util/crc16.h>


/* Host-specific opaque initialization */
#define host_init() // NOP on AVR

/* Host-specific interrupt enable */
#define host_sei() sei()
/* Host-specific interrupt vector declaration */
#define HOST_INTERRUPT(x) ISR(x)

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
#if __AVR_LIBC_VERSION__ >= 10607UL
# define host_nvs_store_byte(a, v) eeprom_update_byte(a, v)
# define host_nvs_store_word(a, v) eeprom_update_word(a, v)
# define host_nvs_store_data(a, v, s) eeprom_update_block(v, a, s)
#else
# define host_nvs_store_byte(a, v) eeprom_write_byte(a, v)
# define host_nvs_store_word(a, v) eeprom_write_word(a, v)
# define host_nvs_store_data(a, v, s) eeprom_write_block(v, a, s)
#endif
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

/* Host GPIO interface */
/* For this architecture, convenience #defines are also provided for Arduino pin
 * names. */
//TODO: fill this up completely
#define HOST_GPIO_PB_port PORTB
#define HOST_GPIO_PB_pin PINB
#define HOST_GPIO_PB_ddr DDRB
#define HOST_GPIO_PB_bit 0 // Dummy
#define HOST_GPIO_PB0_port HOST_GPIO_PB_port
#define HOST_GPIO_PB0_pin HOST_GPIO_PB_pin
#define HOST_GPIO_PB0_ddr HOST_GPIO_PB_ddr
#define HOST_GPIO_PB0_bit 0
#define HOST_GPIO_PB1_port HOST_GPIO_PB_port
#define HOST_GPIO_PB1_pin HOST_GPIO_PB_pin
#define HOST_GPIO_PB1_ddr HOST_GPIO_PB_ddr
#define HOST_GPIO_PB1_bit 1
#define HOST_GPIO_PB1_timer 1
#define HOST_GPIO_PB1_channel A
#define HOST_GPIO_PB3_port HOST_GPIO_PB_port
#define HOST_GPIO_PB3_pin HOST_GPIO_PB_pin
#define HOST_GPIO_PB3_ddr HOST_GPIO_PB_ddr
#define HOST_GPIO_PB3_bit 3
#define HOST_GPIO_PB3_timer 2
#define HOST_GPIO_PB3_channel A
#define HOST_GPIO_PC_port PORTC
#define HOST_GPIO_PC_pin PINC
#define HOST_GPIO_PC_ddr DDRC
#define HOST_GPIO_PC_bit 0 // Dummy
#define HOST_GPIO_PC0_port HOST_GPIO_PC_port
#define HOST_GPIO_PC0_pin HOST_GPIO_PC_pin
#define HOST_GPIO_PC0_ddr HOST_GPIO_PC_ddr
#define HOST_GPIO_PC0_bit 0
#define HOST_GPIO_PC1_port HOST_GPIO_PC_port
#define HOST_GPIO_PC1_pin HOST_GPIO_PC_pin
#define HOST_GPIO_PC1_ddr HOST_GPIO_PC_ddr
#define HOST_GPIO_PC1_bit 1
#define HOST_GPIO_PC2_port HOST_GPIO_PC_port
#define HOST_GPIO_PC2_pin HOST_GPIO_PC_pin
#define HOST_GPIO_PC2_ddr HOST_GPIO_PC_ddr
#define HOST_GPIO_PC2_bit 2
#define HOST_GPIO_PC3_port HOST_GPIO_PC_port
#define HOST_GPIO_PC3_pin HOST_GPIO_PC_pin
#define HOST_GPIO_PC3_ddr HOST_GPIO_PC_ddr
#define HOST_GPIO_PC3_bit 3
#define HOST_GPIO_PC4_port HOST_GPIO_PC_port
#define HOST_GPIO_PC4_pin HOST_GPIO_PC_pin
#define HOST_GPIO_PC4_ddr HOST_GPIO_PC_ddr
#define HOST_GPIO_PC4_bit 4
#define HOST_GPIO_PC5_port HOST_GPIO_PC_port
#define HOST_GPIO_PC5_pin HOST_GPIO_PC_pin
#define HOST_GPIO_PC5_ddr HOST_GPIO_PC_ddr
#define HOST_GPIO_PC5_bit 5
#define HOST_GPIO_PC6_port HOST_GPIO_PC_port
#define HOST_GPIO_PC6_pin HOST_GPIO_PC_pin
#define HOST_GPIO_PC6_ddr HOST_GPIO_PC_ddr
#define HOST_GPIO_PC6_bit 6
#define HOST_GPIO_PC7_port HOST_GPIO_PC_port
#define HOST_GPIO_PC7_pin HOST_GPIO_PC_pin
#define HOST_GPIO_PC7_ddr HOST_GPIO_PC_ddr
#define HOST_GPIO_PC7_bit 7
#define HOST_GPIO_PD_port PORTD
#define HOST_GPIO_PD_pin PIND
#define HOST_GPIO_PD_ddr DDRD
#define HOST_GPIO_PD_bit 0 // Dummy
#define HOST_GPIO_PD2_port HOST_GPIO_PD_port
#define HOST_GPIO_PD2_pin HOST_GPIO_PD_pin
#define HOST_GPIO_PD2_ddr HOST_GPIO_PD_ddr
#define HOST_GPIO_PD2_bit 2
#define HOST_GPIO_PD3_port HOST_GPIO_PD_port
#define HOST_GPIO_PD3_pin HOST_GPIO_PD_pin
#define HOST_GPIO_PD3_ddr HOST_GPIO_PD_ddr
#define HOST_GPIO_PD3_bit 3
#define HOST_GPIO_PD4_port HOST_GPIO_PD_port
#define HOST_GPIO_PD4_pin HOST_GPIO_PD_pin
#define HOST_GPIO_PD4_ddr HOST_GPIO_PD_ddr
#define HOST_GPIO_PD4_bit 4
#define HOST_GPIO_PD5_port HOST_GPIO_PD_port
#define HOST_GPIO_PD5_pin HOST_GPIO_PD_pin
#define HOST_GPIO_PD5_ddr HOST_GPIO_PD_ddr
#define HOST_GPIO_PD5_bit 5
#define HOST_GPIO_PD6_port HOST_GPIO_PD_port
#define HOST_GPIO_PD6_pin HOST_GPIO_PD_pin
#define HOST_GPIO_PD6_ddr HOST_GPIO_PD_ddr
#define HOST_GPIO_PD6_bit 6
#define HOST_GPIO_PD6_timer 0
#define HOST_GPIO_PD6_channel A
/* Host GPIO architecture-specific helpers */
#define __avr_port_of_output(output) output ## _port
#define _avr_port_of_output(output) __avr_port_of_output(output)
#define __avr_pin_of_output(output) output ## _pin
#define _avr_pin_of_output(output) __avr_pin_of_output(output)
#define __avr_ddr_of_output(output) output ## _ddr
#define _avr_ddr_of_output(output) __avr_ddr_of_output(output)
#define _avr_bit_of_output(output) output ## _bit
#define __avr_timer_of_output(output) output ## _timer
#define _avr_timer_of_output(output) __avr_timer_of_output(output)
#define __avr_channel_of_output(output) output ## _channel
#define _avr_channel_of_output(output) __avr_channel_of_output(output)
#define ___avr_ocr_of_output(timer,channel) OCR ## timer ## channel
#define __avr_ocr_of_output(timer,channel) ___avr_ocr_of_output(timer,channel)
#define _avr_ocr_of_output(output) __avr_ocr_of_output(_avr_timer_of_output(output),_avr_channel_of_output(output))
#define ___avr_tccr_of_output(timer,half) TCCR ## timer ## half
#define __avr_tccr_of_output(timer,half) ___avr_tccr_of_output(timer,half)
#define _avr_tccr_of_output(output,half) __avr_tccr_of_output(_avr_timer_of_output(output),half)
#define ___avr_timsk_of_output(timer) TIMSK ## timer
#define __avr_timsk_of_output(timer) ___avr_timsk_of_output(timer)
#define _avr_timsk_of_output(output) __avr_timsk_of_output(_avr_timer_of_output(output))
#define ___avr_com_of_output(timer,channel,bit) COM ## timer ## channel ## bit
#define __avr_com_of_output_(timer,channel,bit) ___avr_com_of_output(timer,channel,bit)
#define _avr_com_of_output(output,bit) __avr_com_of_output_(_avr_timer_of_output(output),_avr_channel_of_output(output),bit)
#define ___avr_wgm_of_output(timer,bit) WGM ## timer ## bit
#define __avr_wgm_of_output(timer,bit) ___avr_wgm_of_output(timer,bit)
#define _avr_wgm_of_output(output,bit) __avr_wgm_of_output(_avr_timer_of_output(output),bit)
#define ___avr_cs_of_output(timer,bit) CS ## timer ## bit
#define __avr_cs_of_output(timer,bit) ___avr_cs_of_output(timer,bit)
#define _avr_cs_of_output(output,bit) __avr_cs_of_output(_avr_timer_of_output(output),bit)
/* Host GPIO interface */
#define host_bit_of_output(output) _avr_bit_of_output(output)
#define host_gpio_direction(output,direction,mode) \
  host_read_modify_write(_avr_ddr_of_output,output,direction,mode)
#define host_gpio_read(output,mode) (mode ? \
    _avr_pin_of_output(output) : \
    (_avr_pin_of_output(output) & _BV(host_bit_of_output(output))))
#define host_gpio_write(output,value,mode) \
  host_read_modify_write(_avr_port_of_output,output,value,mode)
#define host_gpio_toggle(output,mode) (mode ? \
    (_avr_pin_of_output(output) = mode) : \
    (_avr_pin_of_output(output) = _BV(host_bit_of_output(output))))
/* Host GPIO Arduino convenience pin names */
#define HOST_GPIO_ARDUINO_D0 HOST_GPIO_PD0
#define HOST_GPIO_ARDUINO_D1 HOST_GPIO_PD1
#define HOST_GPIO_ARDUINO_D2 HOST_GPIO_PD2
#define HOST_GPIO_ARDUINO_D3 HOST_GPIO_PD3
#define HOST_GPIO_ARDUINO_D4 HOST_GPIO_PD4
#define HOST_GPIO_ARDUINO_D5 HOST_GPIO_PD5
#define HOST_GPIO_ARDUINO_D6 HOST_GPIO_PD6
#define HOST_GPIO_ARDUINO_D7 HOST_GPIO_PD7
#define HOST_GPIO_ARDUINO_D8 HOST_GPIO_PB0
#define HOST_GPIO_ARDUINO_D9 HOST_GPIO_PB1
#define HOST_GPIO_ARDUINO_D10 HOST_GPIO_PB2
#define HOST_GPIO_ARDUINO_D11 HOST_GPIO_PB3
#define HOST_GPIO_ARDUINO_D12 HOST_GPIO_PB4
#define HOST_GPIO_ARDUINO_D13 HOST_GPIO_PB5
#define HOST_GPIO_ARDUINO_A0 HOST_GPIO_PC0
#define HOST_GPIO_ARDUINO_A1 HOST_GPIO_PC1
#define HOST_GPIO_ARDUINO_A2 HOST_GPIO_PC2
#define HOST_GPIO_ARDUINO_A3 HOST_GPIO_PC3
#define HOST_GPIO_ARDUINO_A4 HOST_GPIO_PC4
#define HOST_GPIO_ARDUINO_A5 HOST_GPIO_PC5

/* Host Timer interface */
typedef struct {
    uint16_t divisor;
    uint8_t flags;
} THostTimerPrescaler;
#define HOST_TIMER_CHANNEL_A A
#define HOST_TIMER_CHANNEL_B B
#define HOST_TIMER_INTERRUPT_OVERFLOW_flag __avr_toie_of_timer
#define HOST_TIMER_INTERRUPT_OVERFLOW_vector OVF
#define HOST_TIMER_INTERRUPT_COMPARE_A_flag __avr_ocie_of_timer_helperA
#define HOST_TIMER_INTERRUPT_COMPARE_A_vector COMPA
#define HOST_TIMER_INTERRUPT_COMPARE_B_flag __avr_ocie_of_timer_helperB
#define HOST_TIMER_INTERRUPT_COMPARE_B_vector COMPB
#define HOST_TIMER_PRESCALER_COUNT_0 5
#define HOST_TIMER_PRESCALER_0_0 0x00
#define HOST_TIMER_PRESCALER_0_1 _BV(__avr_cs_of_output(0,0))
#define HOST_TIMER_PRESCALER_0_8 _BV(__avr_cs_of_output(0,1))
#define HOST_TIMER_PRESCALER_0_64 _BV(__avr_cs_of_output(0,0)) | _BV(__avr_cs_of_output(0,1))
#define HOST_TIMER_PRESCALER_0_256 _BV(__avr_cs_of_output(0,2))
#define HOST_TIMER_PRESCALER_0_1024 _BV(__avr_cs_of_output(0,0)) | _BV(__avr_cs_of_output(0,2))
#define HOST_TIMER_PRESCALERS_0 {\
    {1, HOST_TIMER_PRESCALER_0_1},\
    {8, HOST_TIMER_PRESCALER_0_8},\
    {64, HOST_TIMER_PRESCALER_0_64},\
    {256, HOST_TIMER_PRESCALER_0_256},\
    {1024, HOST_TIMER_PRESCALER_0_1024}}
#define HOST_TIMER_PRESCALER_COUNT_1 5
#define HOST_TIMER_PRESCALER_1_0 0x00
#define HOST_TIMER_PRESCALER_1_1 _BV(__avr_cs_of_output(1,0))
#define HOST_TIMER_PRESCALER_1_8 _BV(__avr_cs_of_output(1,1))
#define HOST_TIMER_PRESCALER_1_64 _BV(__avr_cs_of_output(1,0)) | _BV(__avr_cs_of_output(1,1))
#define HOST_TIMER_PRESCALER_1_256 _BV(__avr_cs_of_output(1,2))
#define HOST_TIMER_PRESCALER_1_1024 _BV(__avr_cs_of_output(1,0)) | _BV(__avr_cs_of_output(1,2))
#define HOST_TIMER_PRESCALERS_1 {\
    {1, HOST_TIMER_PRESCALER_1_1},\
    {8, HOST_TIMER_PRESCALER_1_8},\
    {64, HOST_TIMER_PRESCALER_1_64},\
    {256, HOST_TIMER_PRESCALER_1_256},\
    {1024, HOST_TIMER_PRESCALER_1_1024}}
#define HOST_TIMER_PRESCALER_COUNT_2 7
#define HOST_TIMER_PRESCALER_2_0 0x00
#define HOST_TIMER_PRESCALER_2_1 _BV(__avr_cs_of_output(2,0))
#define HOST_TIMER_PRESCALER_2_8 _BV(__avr_cs_of_output(2,1))
#define HOST_TIMER_PRESCALER_2_32 _BV(__avr_cs_of_output(2,0)) | _BV(__avr_cs_of_output(2,1))
#define HOST_TIMER_PRESCALER_2_64 _BV(__avr_cs_of_output(2,2))
#define HOST_TIMER_PRESCALER_2_128 _BV(__avr_cs_of_output(2,0)) | _BV(__avr_cs_of_output(2,2))
#define HOST_TIMER_PRESCALER_2_256 _BV(__avr_cs_of_output(2,1)) | _BV(__avr_cs_of_output(2,2))
#define HOST_TIMER_PRESCALER_2_1024 _BV(__avr_cs_of_output(2,0)) | _BV(__avr_cs_of_output(2,1)) | _BV(__avr_cs_of_output(2,2))
#define HOST_TIMER_PRESCALERS_2 {\
    {1, HOST_TIMER_PRESCALER_2_1},\
    {8, HOST_TIMER_PRESCALER_2_8},\
    {32, HOST_TIMER_PRESCALER_2_32},\
    {64, HOST_TIMER_PRESCALER_2_64},\
    {128, HOST_TIMER_PRESCALER_2_128},\
    {256, HOST_TIMER_PRESCALER_2_256},\
    {1024, HOST_TIMER_PRESCALER_2_1024}}
#define HOST_TIMER_COMPARE_MAX_0 0x100L
#define HOST_TIMER_COMPARE_MAX_1 0x10000L
#define HOST_TIMER_COMPARE_MAX_2 0x100L
#define HOST_TIMER_CTC_0 __avr_tccr_of_output(0,A) |= _BV(__avr_wgm_of_output(0,1))
#define HOST_TIMER_CTC_1 __avr_tccr_of_output(1,B) |= _BV(__avr_wgm_of_output(1,2))
#define HOST_TIMER_CTC_2 __avr_tccr_of_output(2,A) |= _BV(__avr_wgm_of_output(2,1))
#define ___avr_toie_of_timer(timer) TOIE ## timer
#define __avr_toie_of_timer(timer) ___avr_toie_of_timer(timer)
#define ___avr_ocie_of_timer(timer,channel) OCIE ## timer ## channel
#define __avr_ocie_of_timer(timer,channel) ___avr_ocie_of_timer(timer,channel)
#define __avr_ocie_of_timer_helperA(timer) __avr_ocie_of_timer(timer,A)
#define __avr_ocie_of_timer_helperB(timer) __avr_ocie_of_timer(timer,B)
#define host_timer_set_compare(timer,channel,value) __avr_ocr_of_output(timer,channel) = value
#define ___avr_ie_of_timer(timer,which) which ## _ ## timer
#define __avr_ie_of_timer(timer,which) ___avr_ie_of_timer(timer,which)
#define ___avr_if_of_timer(which) which ## _flag
#define __avr_if_of_timer(which) ___avr_if_of_timer(which)
#define _host_timer_enable_interrupt(timer,which) __avr_timsk_of_output(timer) |= _BV(which(timer))
#define host_timer_enable_interrupt(timer,which) _host_timer_enable_interrupt(timer,__avr_if_of_timer(which))
#define _host_timer_disable_interrupt(timer,which) __avr_timsk_of_output(timer) &= ~_BV(which(timer))
#define host_timer_disable_interrupt(timer,which) _host_timer_disable_interrupt(timer,__avr_if_of_timer(which))
#define ___avr_timer_vector_name(timer,which) TIMER ## timer ## _ ## which ## _vect
#define __avr_timer_vector_name(timer,which) ___avr_timer_vector_name(timer,which)
#define ___avr_timer_vector_type(which) which ## _vector
#define __avr_timer_vector_type(which) ___avr_timer_vector_type(which)
#define host_timer_vector_name(timer,which) __avr_timer_vector_name(timer,__avr_timer_vector_type(which))
#define host_timer_set_prescaler(timer,prescaler) \
  __avr_tccr_of_output(timer,B) = (__avr_tccr_of_output(timer,B) & \
      ~(_BV(__avr_cs_of_output(timer,2)) | _BV(__avr_cs_of_output(timer,1)) | \
      _BV(__avr_cs_of_output(timer,0)))) | prescaler
#define _host_prescaler_of_divisor(timer,divisor) HOST_TIMER_PRESCALER_ ## timer ## _ ## divisor
#define host_prescaler_of_divisor(timer,divisor) _host_prescaler_of_divisor(timer,divisor)
#define ___avr_tcnt_of_timer(timer) TCNT ## timer
#define __avr_tcnt_of_timer(timer) ___avr_tcnt_of_timer(timer)
#define host_timer_set_count(timer,count) __avr_tcnt_of_timer(timer) = count
#define _host_timer_enable_ctc(timer) HOST_TIMER_CTC_ ## timer
#define host_timer_enable_ctc(timer) _host_timer_enable_ctc(timer)
/* Sets up given timer for CTC mode for a period of cycles cycles. Actual
 * achievable cycles is returned in actual_cycles. */
#define host_timer_set_reload(timer,cycles,actual_cycles) {\
  uint8_t i; \
  THostTimerPrescaler prescalers[] = host_prescalers_of_timer(timer); \
  uint16_t ceiling = 0; \
  for(i = 0; i < host_prescaler_count_of_timer(timer); i++) {\
    if((cycles) < host_compare_max_of_timer(timer) * prescalers[i].divisor) {\
      ceiling = (cycles) / prescalers[i].divisor; \
      host_timer_set_compare(timer,HOST_TIMER_CHANNEL_A,ceiling); \
      actual_cycles = ceiling * prescalers[i].divisor; \
      host_timer_enable_ctc(timer); \
      host_timer_set_prescaler(timer,prescalers[i].flags); \
      break; \
    }\
  }\
  if(!ceiling) {\
    host_timer_set_compare(timer,HOST_TIMER_CHANNEL_A,host_compare_max_of_timer(timer) - 1); \
    actual_cycles = (host_compare_max_of_timer(timer) - 1) * prescalers[i].divisor; \
    host_timer_enable_ctc(timer); \
    host_timer_set_prescaler(timer,prescalers[i].flags); \
  }}

/* Host waveform generator interface */
/* Starts generating the given waveform at the given frequency on the given
 * output. */
//TODO: Investigate whether instrumenting OCRxB functionality is worth it
#define host_functiongenerator_start(output,frequency,form) {\
  uint32_t ac; \
  host_timer_set_reload(_avr_timer_of_output(output),F_CPU / (frequency * 2),ac); \
  _avr_tccr_of_output(output, A) = _BV(_avr_com_of_output(output, 0)) | _BV(_avr_wgm_of_output(output, 1)); \
  _avr_timsk_of_output(output) = 0; \
  }
/* Stops signal generation (and frees resources, if any used) */
#define host_functiongenerator_stop(output) {\
  _avr_tccr_of_output(output, A) = 0x00; \
  host_timer_set_prescaler(_avr_timer_of_output(output),host_prescaler_of_divisor(_avr_timer_of_output(output),0)); \
  }


#endif /* HOST_HAL_H_ */
